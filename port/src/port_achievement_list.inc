/* The official set as the Options menu lists it: this game's own unlocks
   (achievements_official_eu.dat) and, when the launcher has read the account,
   the account's (cache/achievement-unlocks.dat: public ids only). */
#include "port/port_achievement_list.h"
static unsigned char sAccountUnlocked[KH_OFFLINE_MAX];
static int AchievementReadU32(FILE* file, uint32_t* value) {
    unsigned char b[4];
    if (fread(b, 1, 4, file) != 4) return 0;
    *value = b[0] | (uint32_t)b[1]<<8 | (uint32_t)b[2]<<16 | (uint32_t)b[3]<<24;
    return 1;
}
void PortAchievementsListRefresh(void) {
    FILE* file;
    char signature[8];
    uint32_t count, known, id;
    int i, index;
    memset(sAccountUnlocked, 0, sizeof(sAccountUnlocked));
    file = fopen(PortConfigPath("cache/achievement-unlocks.dat"), "rb");
    if (!file) return;
    if (fread(signature, 1, 8, file) == 8 && !memcmp(signature, "KHRAPRG1", 8) &&
        AchievementReadU32(file, &known) && known == 1 && AchievementReadU32(file, &count) && count <= 2000)
        for (i = 0; i < (int)count && AchievementReadU32(file, &id); ++i)
            if ((index = KhOfficialIndex(id)) >= 0) sAccountUnlocked[index] = 1;
    fclose(file);
}
int PortAchievementsListCount(void) { return KhOfficialCount(); }
int PortAchievementsListGet(int index, PortAchievementInfo* out) {
    const KhOfficialAchievement* a = KhOfficialAt(index);
    if (!out || !a) return 0;
    SDL_strlcpy(out->title, a->title, sizeof(out->title));
    SDL_strlcpy(out->description, a->description, sizeof(out->description));
    if (SDL_strlen(a->badge) < sizeof(out->badge)) SDL_strlcpy(out->badge, a->badge, sizeof(out->badge));
    else out->badge[0] = 0;
    out->points = a->points;
    out->state = sAccountUnlocked[index];
#ifdef KH_OFFLINE_EVAL
    out->state |= sOffline.when[index] != 0;
#endif
    return 1;
}
