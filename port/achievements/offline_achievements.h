#ifndef KH_OFFLINE_ACHIEVEMENTS_H
#define KH_OFFLINE_ACHIEVEMENTS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The official set (European cartridge), evaluated by the game itself so it
 * works with no account: unlocks are kept in achievements_official_eu.dat in
 * the data folder (a line per unlock: id and Unix time). */
/* languages in the port's order: English, French, German, Italian, Spanish, Japanese */
#define KH_OFFICIAL_LANGS 6
typedef struct KhOfficialAchievement {
    uint32_t id, points;
    const char *badge, *definition;
    const char* text[KH_OFFICIAL_LANGS][2]; /* title, description */
} KhOfficialAchievement;
/* the title or description (field 0 or 1) in that language; English if out of range */
const char* KhOfficialText(const KhOfficialAchievement* a, int lang, int field);

int KhOfficialCount(void);
const KhOfficialAchievement* KhOfficialAt(int index);
int KhOfficialIndex(uint32_t id); /* -1 if not in the set */

#define KH_OFFLINE_MAX 128
typedef struct KhOfflineState {
    uint64_t when[KH_OFFLINE_MAX]; /* by set index; 0 locked, else the unlock time (1 if unknown) */
} KhOfflineState;

int KhOfflineLoad(KhOfflineState* state, const char* path);
int KhOfflineSave(const KhOfflineState* state, const char* path);
int KhOfflineUnlockedCount(const KhOfflineState* state);

#ifdef __cplusplus
}
#endif
#endif
