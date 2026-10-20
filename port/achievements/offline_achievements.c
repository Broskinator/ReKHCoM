#include "offline_achievements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "official_set_eu.inc"

int KhOfficialCount(void) {
    return KH_OFFICIAL_COUNT;
}

const KhOfficialAchievement* KhOfficialAt(int index) {
    return index >= 0 && index < KH_OFFICIAL_COUNT ? &gKhOfficialSet[index] : NULL;
}

int KhOfficialIndex(uint32_t id) {
    int i;
    for (i = 0; i < KH_OFFICIAL_COUNT; ++i)
        if (gKhOfficialSet[i].id == id) return i;
    return -1;
}

static FILE* Open(const char* path, const char* mode) {
#ifdef _WIN32
    wchar_t wide[1400], wmode[8];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 1400) ||
        !MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 8)) return NULL;
    return _wfopen(wide, wmode);
#else
    return fopen(path, mode);
#endif
}

/* "KHOFF1", then "<id> <time>" per unlock; unknown ids are skipped, so a
 * later set keeps what the player already earned */
int KhOfflineLoad(KhOfflineState* state, const char* path) {
    char line[96];
    FILE* file = Open(path, "r");
    memset(state, 0, sizeof(*state));
    if (!file) return 0;
    if (!fgets(line, sizeof(line), file) || strncmp(line, "KHOFF1", 6) != 0) {
        fclose(file);
        return 0;
    }
    while (fgets(line, sizeof(line), file)) {
        char* end;
        unsigned long id = strtoul(line, &end, 10);
        unsigned long long when = strtoull(end, NULL, 10);
        int index = end != line ? KhOfficialIndex((uint32_t)id) : -1;
        if (index >= 0) state->when[index] = when ? when : 1;
    }
    fclose(file);
    return 1;
}

int KhOfflineSave(const KhOfflineState* state, const char* path) {
    char temp[1400];
    FILE* file;
    int i, ok;
    if (snprintf(temp, sizeof(temp), "%s.tmp", path) >= (int)sizeof(temp)) return 0;
    file = Open(temp, "w");
    if (!file) return 0;
    ok = fprintf(file, "KHOFF1\n") > 0;
    for (i = 0; ok && i < KH_OFFICIAL_COUNT; ++i)
        if (state->when[i])
            ok = fprintf(file, "%lu %llu\n", (unsigned long)gKhOfficialSet[i].id, (unsigned long long)state->when[i]) > 0;
    if (fclose(file) != 0) ok = 0;
    if (!ok) return 0;
#ifdef _WIN32
    {
        wchar_t from[1400], to[1400];
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, temp, -1, from, 1400) ||
            !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, to, 1400)) return 0;
        return MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    }
#else
    return rename(temp, path) == 0;
#endif
}

int KhOfflineUnlockedCount(const KhOfflineState* state) {
    int i, n = 0;
    for (i = 0; i < KH_OFFICIAL_COUNT; ++i) n += state->when[i] != 0;
    return n;
}
