/*
 * HD music (port/progress/HD_MOD.md): the Re:Chain of Memories soundtrack of the player's own KINGDOM HEARTS -HD
 * 1.5+2.5 ReMIX-, imported by the launcher as hd/music/<track>.ogg (+ .loop: start and end sample), plays in place
 * of the cartridge's background music.
 *
 * The game keeps running its own song: this only watches the BGM player (which song, paused, stopped, fading),
 * silences its channels (gPortMix.musicGain) and streams the matching track at 48 kHz on an audio stream of its own,
 * so the track is never resampled down to the GBA's rate. Off with hd_music=0, or when no track was imported.
 *
 * Desktop and 32-bit Android (hosted C library) decode the track here. The bridge images (64-bit Android, Switch,
 * Vita, PS4, PS5) have no C library for it: they hand the track to their host (bridge entry hd_music,
 * port/ps5/shared/hd_music_host.h), which mixes it into the game's sound. The web gets the empty functions below.
 */
#include "port/port.h"
#include "port/port_ui.h"

#if !defined(__EMSCRIPTEN__)
#include "lib/m4a.h"
#include "port/port_audio.h"

extern MusicPlayerInfo gMPlayInfo_BGM;

/* Cartridge song (include/songs.h, 1..43) → Re:CoM track (STREAM/0001/<name>.win32.scd). */
static const char* const kTracks[44] = {
    NULL,
    "21_Alice_Battle", "19_Alice_Field", "Boss1", "Event2_", "Event1_Kinpak", "Title", "12_Event_Yuttari",
    "24_Pinocchio_Field", "27_Pinocchio_B", "22_PeterPan_Field", "23_PeterPan_Battle", "01F_Town_B", "01F_Town_F",
    "16_XIIIBoss1", "17_Halloween_F", "18_Helloween_B", "39_LittleMermaid_B", "15_Event_XIII", "13_Event_Namine",
    "03F_Hercules_B", "03F_Hercul", "38_UnderTheSea", "26_Aladdin_B", "Event4", "42_Hollow_F", "43_Hollow_B",
    "14_Event_Odayaka", "Event_Unrest", "34_Boss_RikuAnsem", "20_Event_13thFloor", "25_Aladdin_F",
    "36_WinnieThePooh", "32_Destiny_F", "44_PooGame1", "35_Boss_NiseRiku", "30_Twilight_F", "37_LastBoss1",
    "28_Forget_F", "29_Forget_B", "33_Destiny_B", "Boss2_World", "40_LastBoss2", "31_Twilight_B",
};

static int sSong = -1, sMissing = -1; /* sMissing: the song whose track did not open (tried once) */

const char* PortHdMusicTrack(int song) {
    return song > 0 && song < (int)(sizeof(kTracks) / sizeof(kTracks[0])) ? kTracks[song] : NULL;
}

/* The cartridge song the BGM player has, or -1. */
static int CurrentSong(void) {
    const MusicPlayerInfo* bgm = &gMPlayInfo_BGM;
    int i;
    if (bgm->songHeader == NULL)
        return -1;
    for (i = 1; i < (int)(sizeof(kTracks) / sizeof(kTracks[0])); i++)
        if (gSongTable[i].header == bgm->songHeader)
            return i;
    return -1;
}

#if __STDC_HOSTED__
/* ---- hosted: stb_vorbis on an SDL audio stream of its own --------------------------------------------------- */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_INTEGER_CONVERSION
#include "../third_party/stb/stb_vorbis.c"

static SDL_AudioStream* sStream;
static stb_vorbis* sVorbis;
static int sRate;
static unsigned sLoopStart, sLoopEnd, sPosition;
static unsigned char* sFile;

static void Close(void) {
    if (sVorbis)
        stb_vorbis_close(sVorbis);
    sVorbis = NULL;
    SDL_free(sFile);
    sFile = NULL;
    if (sStream)
        SDL_ClearAudioStream(sStream);
    sSong = -1;
}

static int Open(int song) {
    char name[160];
    size_t size = 0;
    int error = 0;
    FILE* loop;

    Close();
    SDL_snprintf(name, sizeof(name), "hd/music/%s.ogg", kTracks[song]);
    sFile = SDL_LoadFile(PortConfigPath(name), &size);
    if (sFile == NULL)
        return 0;
    sVorbis = stb_vorbis_open_memory(sFile, (int)size, &error, NULL);
    if (sVorbis == NULL) {
        PortLog("hd music: %s does not decode (%d)", name, error);
        Close();
        return 0;
    }
    {
        stb_vorbis_info info = stb_vorbis_get_info(sVorbis);
        sRate = (int)info.sample_rate;
    }
    sLoopStart = 0;
    sLoopEnd = stb_vorbis_stream_length_in_samples(sVorbis);
    SDL_snprintf(name, sizeof(name), "hd/music/%s.loop", kTracks[song]);
    loop = fopen(PortConfigPath(name), "r");
    if (loop != NULL) {
        unsigned a, b;
        if (fscanf(loop, "%u %u", &a, &b) == 2 && a < b && b <= sLoopEnd) {
            sLoopStart = a;
            sLoopEnd = b;
        }
        fclose(loop);
    }
    sPosition = 0;
    if (sStream == NULL) {
        SDL_AudioSpec spec = { SDL_AUDIO_F32, 2, sRate };
        sStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (sStream)
            SDL_ResumeAudioStreamDevice(sStream);
    } else {
        SDL_AudioSpec spec = { SDL_AUDIO_F32, 2, sRate };
        SDL_SetAudioStreamFormat(sStream, &spec, NULL);
    }
    sSong = song;
    PortLog("hd music: song %d -> %s", song, kTracks[song]);
    return sStream != NULL;
}

/* Decodes until about a fifth of a second is queued, looping at the track's loop points. */
static void Feed(float gain) {
    float pcm[2048 * 2];
    while (SDL_GetAudioStreamQueued(sStream) < (int)(sRate / 5 * 2 * sizeof(float))) {
        int want = 2048, got, i;
        if (sPosition + (unsigned)want > sLoopEnd)
            want = (int)(sLoopEnd - sPosition);
        if (want <= 0) {
            stb_vorbis_seek(sVorbis, sLoopStart);
            sPosition = sLoopStart;
            continue;
        }
        got = stb_vorbis_get_samples_float_interleaved(sVorbis, 2, pcm, want * 2);
        if (got <= 0) {
            stb_vorbis_seek(sVorbis, sLoopStart);
            sPosition = sLoopStart;
            continue;
        }
        for (i = 0; i < got * 2; i++)
            pcm[i] *= gain;
        SDL_PutAudioStreamData(sStream, pcm, got * 2 * (int)sizeof(float));
        sPosition += (unsigned)got;
    }
}

static void Pause(void) {
    if (sStream)
        SDL_ClearAudioStream(sStream); /* the track resumes from where it was */
}

static void Shutdown(void) {
    Close();
    if (sStream)
        SDL_DestroyAudioStream(sStream);
    sStream = NULL;
}
#else
/* ---- bridge images: the host decodes and mixes (port/ps5/shared/hd_music_host.h) ----------------------------- */
#include <stdio.h>

#include "bridge_calls.h"

enum { HD_MUSIC_OPEN, HD_MUSIC_PLAY, HD_MUSIC_PAUSE, HD_MUSIC_CLOSE };

static void Close(void) {
    if (sSong >= 0)
        h_hd_music(HD_MUSIC_CLOSE, "", 0);
    sSong = -1;
}

static int Open(int song) {
    char name[160];
    Close();
    snprintf(name, sizeof(name), "hd/music/%s.ogg", kTracks[song]);
    if (!h_hd_music(HD_MUSIC_OPEN, PortConfigPath(name), 0))
        return 0;
    sSong = song;
    PortLog("hd music: song %d -> %s", song, kTracks[song]);
    return 1;
}

static void Feed(float gain) {
    h_hd_music(HD_MUSIC_PLAY, "", (int)(gain * 1000.0f));
}

static void Pause(void) {
    if (sSong >= 0)
        h_hd_music(HD_MUSIC_PAUSE, "", 0);
}

static void Shutdown(void) {
    Close();
}
#endif

/* Once per frame, after the mixer gains are set (port_audio.c). */
void PortHdMusicFrame(void) {
    const MusicPlayerInfo* bgm = &gMPlayInfo_BGM;
    int song, playing;
    float gain;

    if (!gPortConfig.hdMusic) {
        if (sSong >= 0)
            Close();
        return;
    }
    song = CurrentSong();
    playing = song > 0 && (bgm->status & MUSICPLAYER_STATUS_TRACK) && !(bgm->status & MUSICPLAYER_STATUS_PAUSE);
    if (!playing || PortUiActive()) {
        Pause(); /* stopped or paused: the track resumes from where it was */
        if (song != sSong && sSong >= 0 && !(song > 0 && (bgm->status & MUSICPLAYER_STATUS_PAUSE)))
            Close();
        return;
    }
    if (song != sSong) {
        if (song == sMissing)
            return; /* not imported: the cartridge's song plays */
        if (!Open(song)) {
            PortLog("hd music: no track for song %d (%s)", song, kTracks[song]);
            sMissing = song;
            return;
        }
        sMissing = -1;
    }
    gain = gPortMix.musicGain * gPortConfig.masterVolume / 100.0f;
    if (bgm->fadeOI)
        gain *= (float)(bgm->fadeOV >> FADE_VOL_SHIFT) / FADE_VOL_MAX;
    gPortMix.musicGain = 0.0f; /* the cartridge's song keeps playing, silent */
    Feed(gain);
}

void PortHdMusicShutdown(void) {
    Shutdown();
}
#else
void PortHdMusicFrame(void) {
}
void PortHdMusicShutdown(void) {
}
const char* PortHdMusicTrack(int song) {
    (void)song;
    return NULL;
}
#endif
