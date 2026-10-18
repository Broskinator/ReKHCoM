/* Japanese battle voices from an independently verified original cartridge.
 * No patch or audio data is shipped. The map contains addresses and lengths.
 * Keep samples at fixed ROM addresses so save-state pointers remain valid. */
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port/port.h"

typedef struct { uint32_t pointer, dest, source, size; } UndubSample;
typedef struct { uint32_t dest, source; int header, oldHeader; } UndubSong;
#include "port/port_undub_map.h"

#ifndef VERSION_JP
static uint8_t* sJapanese;
static int sChecked;
#endif

static uint32_t Read32(const uint8_t* p) {
    return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void Write32(uint8_t* p, uint32_t n) {
    p[0] = n; p[1] = n >> 8; p[2] = n >> 16; p[3] = n >> 24;
}
static uint32_t Crc32(const uint8_t* p, size_t n) {
    uint32_t table[256], crc = ~0u;
    unsigned i, j;
    for (i = 0; i < 256; i++) {
        uint32_t c = i;
        for (j = 0; j < 8; j++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1)));
        table[i] = c;
    }
    for (i = 0; i < n; i++) crc = (crc >> 8) ^ table[(crc ^ p[i]) & 255];
    return ~crc;
}

int PortUndubAvailable(void) {
#ifdef VERSION_JP
    return 0; /* already Japanese */
#else
    static const char* names[] = { "roms/B8CJ.gba", "../../roms/B8CJ.gba" };
    size_t i;
    if (sChecked) return sJapanese != NULL;
    sChecked = 1;
    for (i = 0; i < SDL_arraysize(names); i++) {
        FILE* f = fopen(PortConfigPath(names[i]), "rb");
        if (!f) continue;
        sJapanese = malloc(GBA_ROM_SIZE);
        if (!sJapanese) { fclose(f); return 0; }
        int ok = fread(sJapanese, 1, GBA_ROM_SIZE, f) == GBA_ROM_SIZE && fgetc(f) == EOF;
        fclose(f);
        if (ok && memcmp(sJapanese + 0xAC, "B8CJ", 4) == 0 && Crc32(sJapanese, GBA_ROM_SIZE) == 0x8D5A0D84u)
            return 1;
        free(sJapanese); sJapanese = NULL;
    }
    return 0;
#endif
}

void PortUndubApply(void) {
#ifndef VERSION_JP
    uint8_t* rom = (uint8_t*)(uintptr_t)GBA_ROM;
    size_t i;
    if (!gPortConfig.undub) return;
    if (!PortUndubAvailable()) { PortLog("undub: original Japanese ROM unavailable; original audio retained"); return; }
    for (i = 0; i < SDL_arraysize(kUndubSamples); i++) {
        const UndubSample* s = &kUndubSamples[i];
        if (s->source) memcpy(rom + s->dest, sJapanese + s->source, s->size);
        else memset(rom + s->dest, 0, s->size); /* removed English-only voice */
        Write32(rom + s->pointer, GBA_ROM + s->dest);
    }
    for (i = 0; i < SDL_arraysize(kUndubSongs); i++) {
        const UndubSong* s = &kUndubSongs[i];
        const uint8_t* from = sJapanese + s->source;
        uint8_t* to = rom + s->dest;
        int k, out = 0, running = -1;
        /* Voice tracks contain only single-byte MIDI commands/arguments.
         * Remove redundant note opcodes and centre-pan commands, as Undub
         * does. The original header stays pinned for native gSongTable. */
        for (k = 0; k < s->header; k++) {
            int b = from[k];
            if (b == 0xBF && k + 1 < s->header && from[k + 1] == 0x40) { k++; continue; }
            if (b >= 0xCF && b == running) continue;
            if (b >= 0xBD) running = b;
            to[out++] = b;
            if (b == 0xB1) break;
        }
        memset(to + out, 0, s->oldHeader - out);
        memcpy(to + s->oldHeader, from + s->header, 12);
        /* The voice group and track pointers are relative to each region. */
#ifdef VERSION_US
        const uint32_t groupDelta = 0x26AF4;
#else
        const uint32_t groupDelta = 0x96A20;
#endif
        Write32(to + s->oldHeader + 4, Read32(from + s->header + 4) + groupDelta);
        Write32(to + s->oldHeader + 8, GBA_ROM + s->dest);
    }
    PortLog("undub: %u Japanese samples, %u voice tracks loaded", (unsigned)SDL_arraysize(kUndubSamples),
            (unsigned)SDL_arraysize(kUndubSongs));
#endif
}
