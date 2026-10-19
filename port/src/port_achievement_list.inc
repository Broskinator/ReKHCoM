/* The launcher cache contains public metadata and last-query unlock IDs only. */
#include "port/port_achievement_list.h"
static struct {
    uint32_t id;
    PortAchievementInfo info;
} sOfficialList[256];
static int sOfficialCount;
static int AchievementReadU32(FILE* file, uint32_t* value) {
    unsigned char b[4];
    if (fread(b, 1, 4, file) != 4) return 0;
    *value = b[0] | (uint32_t)b[1]<<8 | (uint32_t)b[2]<<16 | (uint32_t)b[3]<<24;
    return 1;
}
static int AchievementReadString(FILE* file, char* out, size_t capacity) {
    uint32_t length;
    size_t keep;
    if (!AchievementReadU32(file, &length) || length > 16384) return 0;
    keep = length < capacity ? length : capacity - 1;
    if (fread(out, 1, keep, file) != keep) return 0;
    out[keep] = 0;
    return !length || fseek(file, (long)(length - keep), SEEK_CUR) == 0;
}
void PortAchievementsListRefresh(void) {
    FILE* file;
    char signature[8], ignored[40];
    uint32_t count, points, known, id;
    int i, j;
    sOfficialCount = 0;
    file = fopen(PortConfigPath("cache/achievements-768.dat"), "rb");
    if (!file) return;
    if (fread(signature, 1, 8, file) != 8 || memcmp(signature, "KHRACAT1", 8) ||
        !AchievementReadU32(file, &count) || count > 256) { fclose(file); return; }
    for (i = 0; i < (int)count; ++i) {
        PortAchievementInfo* info = &sOfficialList[i].info;
        if (!AchievementReadU32(file, &sOfficialList[i].id) || !AchievementReadU32(file, &points) ||
            !AchievementReadString(file, info->title, sizeof(info->title)) ||
            !AchievementReadString(file, info->description, sizeof(info->description)) ||
            !AchievementReadString(file, ignored, sizeof(ignored))) { fclose(file); return; }
        info->state = -1;
    }
    fclose(file); sOfficialCount = (int)count;
    file = fopen(PortConfigPath("cache/achievement-unlocks.dat"), "rb");
    if (!file) return;
    if (fread(signature, 1, 8, file) != 8 || memcmp(signature, "KHRAPRG1", 8) ||
        !AchievementReadU32(file, &known) || known != 1 ||
        !AchievementReadU32(file, &count) || count > 2000) { fclose(file); return; }
    /* A truncated progress snapshot leaves every account state unknown. */
    for (i = 0; i < (int)count; ++i) {
        if (!AchievementReadU32(file, &id)) {
            for (j = 0; j < sOfficialCount; ++j) sOfficialList[j].info.state = -1;
            fclose(file); return;
        }
        for (j = 0; j < sOfficialCount; ++j) if (sOfficialList[j].id == id) sOfficialList[j].info.state = 1;
    }
    for (j = 0; j < sOfficialCount; ++j) if (sOfficialList[j].info.state < 0) sOfficialList[j].info.state = 0;
    fclose(file);
}
int PortAchievementsListCount(int official) { return official ? sOfficialCount : KH_LOCAL_COUNT; }
int PortAchievementsListGet(int official, int index, int spanish, PortAchievementInfo* out) {
    const KhLocalDefinition* def;
    if (!out || index < 0 || index >= PortAchievementsListCount(official)) return 0;
    if (official) { *out = sOfficialList[index].info; return 1; }
    def = &gKhLocalDefinitions[index];
    SDL_strlcpy(out->title, spanish ? def->title_es : def->title_en, sizeof(out->title));
    SDL_strlcpy(out->description, spanish ? def->description_es : def->description_en, sizeof(out->description));
    out->state = (sLocal.unlocked & (1u << index)) != 0;
    return 1;
}
