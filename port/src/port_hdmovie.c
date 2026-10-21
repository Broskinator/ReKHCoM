/*
 * HD movies (port/progress/HD_MOD.md): the Re:Chain of Memories movies of the player's own KINGDOM HEARTS -HD
 * 1.5+2.5 ReMIX-, imported by the launcher as hd/movies/<name>.mp4, play in place of the cartridge's movies.
 *
 * mode_movie.c calls PortHdMoviePlay when a movie starts: if the movie has an imported file and hd_movies is on, the
 * whole movie plays here (the window's renderer, an audio stream of its own) and the game goes on as when its own
 * movie ends; Start, A, Enter, Escape or a tap skip it. Otherwise the cartridge's movie plays as always.
 *
 * Windows: Media Foundation (part of Windows: H.264 and AAC need nothing else). Other systems: not yet (returns 0).
 */
#include "port/port.h"

#ifdef _WIN32
#define COBJMACROS
#include <SDL3/SDL.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

SDL_Renderer* PortRenderer(void);

/* include/evt/mode_movie.h: MOVIE_OPENING 1, MOVIE_6F_GOAL, MOVIE_12F_E2, MOVIE_ENDING, MOVIE_RIKU_ENDING */
static const char* MovieFile(int movieId) {
    switch (movieId) {
        case 1: return "ROP";  /* the opening */
        case 4: return "RSE";  /* Sora's ending */
        case 5: return "RRE";  /* Riku's ending (Reverse/Rebirth) */
        default: return NULL;  /* the 6th and 12th floor movies: the full mod's scenes (hd_scenes), later */
    }
}

static int SkipPressed(int* quit) {
    SDL_Event e;
    int skip = 0;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                *quit = 1;
                return 1;
            case SDL_EVENT_KEY_DOWN:
                if (!e.key.repeat && (e.key.key == SDLK_RETURN || e.key.key == SDLK_ESCAPE || e.key.key == SDLK_SPACE ||
                                      e.key.key == SDLK_X || e.key.key == SDLK_Z))
                    skip = 1;
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                if (e.gbutton.button == SDL_GAMEPAD_BUTTON_START || e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH ||
                    e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)
                    skip = 1;
                break;
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                skip = 1;
                break;
            default:
                break;
        }
    }
    return skip;
}

/* Plays hd/movies/<name>.mp4 to the end (or until skipped). 0: nothing imported or it could not be opened. */
int PortHdMoviePlay(int movieId) {
    const char* name = MovieFile(movieId);
    char path[1024];
    WCHAR wpath[1024];
    IMFSourceReader* reader = NULL;
    IMFMediaType* type = NULL;
    IMFAttributes* attrs = NULL;
    SDL_Renderer* renderer = PortRenderer();
    SDL_Texture* texture = NULL;
    SDL_AudioStream* audio = NULL;
    UINT32 width = 0, height = 0, rate = 48000, channels = 2;
    LONGLONG audioPushed = 0; /* 100 ns units of audio handed to the stream */
    LONGLONG nextVideo = -1;
    IMFSample* pending = NULL;
    int quit = 0, videoDone = 0, audioDone = 0, played = 0, shown = 0;
    HRESULT hr;

    if (!gPortConfig.hdMovies || name == NULL || renderer == NULL)
        return 0;
    SDL_snprintf(path, sizeof(path), "%s", PortConfigPath("hd/movies/"));
    SDL_strlcat(path, name, sizeof(path));
    SDL_strlcat(path, ".mp4", sizeof(path));
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
        return 0;
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 1024);

    if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE)))
        return 0;
    MFCreateAttributes(&attrs, 1);
    IMFAttributes_SetUINT32(attrs, &MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    hr = MFCreateSourceReaderFromURL(wpath, attrs, &reader);
    IMFAttributes_Release(attrs);
    if (FAILED(hr)) {
        PortLog("hd movie: %s does not open (0x%08lx)", path, (unsigned long)hr);
        MFShutdown();
        return 0;
    }
    /* video as 32-bit BGRX, audio as 16-bit PCM */
    MFCreateMediaType(&type);
    IMFMediaType_SetGUID(type, &MF_MT_MAJOR_TYPE, &MFMediaType_Video);
    IMFMediaType_SetGUID(type, &MF_MT_SUBTYPE, &MFVideoFormat_RGB32);
    hr = IMFSourceReader_SetCurrentMediaType(reader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
    IMFMediaType_Release(type);
    if (SUCCEEDED(hr)) {
        IMFMediaType* actual = NULL;
        if (SUCCEEDED(IMFSourceReader_GetCurrentMediaType(reader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actual))) {
            UINT64 size = 0;
            IMFMediaType_GetUINT64(actual, &MF_MT_FRAME_SIZE, &size);
            width = (UINT32)(size >> 32);
            height = (UINT32)size;
            IMFMediaType_Release(actual);
        }
    }
    MFCreateMediaType(&type);
    IMFMediaType_SetGUID(type, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio);
    IMFMediaType_SetGUID(type, &MF_MT_SUBTYPE, &MFAudioFormat_PCM);
    IMFMediaType_SetUINT32(type, &MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    if (FAILED(IMFSourceReader_SetCurrentMediaType(reader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, type)))
        audioDone = 1;
    IMFMediaType_Release(type);
    if (!audioDone) {
        IMFMediaType* actual = NULL;
        if (SUCCEEDED(IMFSourceReader_GetCurrentMediaType(reader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &actual))) {
            IMFMediaType_GetUINT32(actual, &MF_MT_AUDIO_SAMPLES_PER_SECOND, &rate);
            IMFMediaType_GetUINT32(actual, &MF_MT_AUDIO_NUM_CHANNELS, &channels);
            IMFMediaType_Release(actual);
        }
        {
            SDL_AudioSpec spec = { SDL_AUDIO_S16, (int)channels, (int)rate };
            audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
            if (audio)
                SDL_ResumeAudioStreamDevice(audio);
            else
                audioDone = 1;
        }
    }
    if (width == 0 || height == 0) {
        PortLog("hd movie: %s has no usable video", path);
        goto done;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, (int)width, (int)height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    PortLog("hd movie: %s (%ux%u, %u Hz)", name, width, height, rate);
    played = 1;

    while (!quit && !(videoDone && audioDone)) {
        Uint64 frameStart = SDL_GetTicksNS();
        LONGLONG clock;
        if (SkipPressed(&quit))
            break;
        /* keep about half a second of audio queued: it is the clock */
        while (!audioDone && audio && SDL_GetAudioStreamQueued(audio) < (int)(rate * channels * 2 / 2)) {
            DWORD flags = 0;
            LONGLONG ts = 0;
            IMFSample* sample = NULL;
            hr = IMFSourceReader_ReadSample(reader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &flags, &ts, &sample);
            if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
                audioDone = 1;
                if (sample)
                    IMFSample_Release(sample);
                break;
            }
            if (sample) {
                IMFMediaBuffer* buffer = NULL;
                if (SUCCEEDED(IMFSample_ConvertToContiguousBuffer(sample, &buffer))) {
                    BYTE* data;
                    DWORD length = 0;
                    if (SUCCEEDED(IMFMediaBuffer_Lock(buffer, &data, NULL, &length))) {
                        SDL_PutAudioStreamData(audio, data, (int)length);
                        audioPushed += (LONGLONG)length * 10000000 / (rate * channels * 2);
                        IMFMediaBuffer_Unlock(buffer);
                    }
                    IMFMediaBuffer_Release(buffer);
                }
                IMFSample_Release(sample);
            }
        }
        /* the presentation time: audio played so far, or the wall clock without audio */
        if (audio && !audioDone)
            clock = audioPushed - (LONGLONG)SDL_GetAudioStreamQueued(audio) * 10000000 / (rate * channels * 2);
        else
            clock = nextVideo >= 0 ? nextVideo : 0;
        /* show the latest video frame due by now */
        while (!videoDone) {
            if (!pending) {
                DWORD flags = 0;
                LONGLONG ts = 0;
                hr = IMFSourceReader_ReadSample(reader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, &flags, &ts, &pending);
                if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
                    videoDone = 1;
                    break;
                }
                if (!pending)
                    continue;
                nextVideo = ts;
            }
            if (nextVideo > clock + 100000) /* more than 10 ms ahead: later */
                break;
            {
                IMFMediaBuffer* buffer = NULL;
                IMF2DBuffer* buffer2d = NULL;
                if (SUCCEEDED(IMFSample_GetBufferByIndex(pending, 0, &buffer))) {
                    BYTE* scan0;
                    LONG pitch;
                    if (SUCCEEDED(IMFMediaBuffer_QueryInterface(buffer, &IID_IMF2DBuffer, (void**)&buffer2d)) &&
                        SUCCEEDED(IMF2DBuffer_Lock2D(buffer2d, &scan0, &pitch))) {
                        if (pitch >= 0) {
                            SDL_UpdateTexture(texture, NULL, scan0, pitch);
                        } else { /* bottom-up rows */
                            void* pixels;
                            int texPitch;
                            if (SDL_LockTexture(texture, NULL, &pixels, &texPitch)) {
                                UINT32 y;
                                for (y = 0; y < height; y++)
                                    memcpy((BYTE*)pixels + (size_t)y * texPitch, scan0 + (LONG)y * pitch, width * 4);
                                SDL_UnlockTexture(texture);
                            }
                        }
                        IMF2DBuffer_Unlock2D(buffer2d);
                    } else {
                        /* a plain buffer (the video processor's output): top-down rows of width * 4 bytes */
                        BYTE* data;
                        DWORD length = 0;
                        if (SUCCEEDED(IMFMediaBuffer_Lock(buffer, &data, NULL, &length))) {
                            if (length >= width * height * 4)
                                SDL_UpdateTexture(texture, NULL, data, (int)(width * 4));
                            IMFMediaBuffer_Unlock(buffer);
                        }
                    }
                    if (buffer2d)
                        IMF2DBuffer_Release(buffer2d);
                    IMFMediaBuffer_Release(buffer);
                }
            }
            IMFSample_Release(pending);
            pending = NULL;
            shown++;
            if (!audio || audioDone)
                break; /* without audio: one frame per loop, paced below */
        }
        {
            int ow = 0, oh = 0;
            SDL_FRect dst;
            float scale;
            SDL_GetCurrentRenderOutputSize(renderer, &ow, &oh);
            scale = SDL_min((float)ow / width, (float)oh / height);
            dst.w = width * scale;
            dst.h = height * scale;
            dst.x = (ow - dst.w) / 2;
            dst.y = (oh - dst.h) / 2;
            SDL_SetRenderViewport(renderer, NULL);
            SDL_SetRenderClipRect(renderer, NULL);
            SDL_SetRenderScale(renderer, 1.0f, 1.0f);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_RenderTexture(renderer, texture, NULL, &dst);
            {
                /* test harness: KHCOM_HDMOVIE_SHOT=<seconds> saves that moment as hdmovie.bmp */
                static int shot;
                const char* at = SDL_getenv("KHCOM_HDMOVIE_SHOT");
                if (at && !shot && clock >= (LONGLONG)SDL_atoi(at) * 10000000) {
                    SDL_Surface* s = SDL_RenderReadPixels(renderer, NULL);
                    if (s) {
                        SDL_SaveBMP(s, PortConfigPath("hdmovie.bmp"));
                        SDL_DestroySurface(s);
                    }
                    shot = 1;
                }
            }
            SDL_RenderPresent(renderer);
        }
        {
            Uint64 spent = SDL_GetTicksNS() - frameStart;
            if (spent < 16000000)
                SDL_DelayNS(16000000 - spent);
        }
        if (audioDone && audio && SDL_GetAudioStreamQueued(audio) > 0 && videoDone)
            while (SDL_GetAudioStreamQueued(audio) > 0 && !SkipPressed(&quit))
                SDL_Delay(10);
    }

done:
    PortLog("hd movie: %d frames shown%s", shown, quit ? ", window closed" : "");
    if (pending)
        IMFSample_Release(pending);
    if (texture)
        SDL_DestroyTexture(texture);
    if (audio)
        SDL_DestroyAudioStream(audio);
    if (reader)
        IMFSourceReader_Release(reader);
    MFShutdown();
    if (quit) {
        SDL_Event q;
        SDL_zero(q);
        q.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&q); /* the main loop closes the game as usual */
    }
    return played;
}
#else
int PortHdMoviePlay(int movieId) {
    (void)movieId;
    return 0;
}
#endif
