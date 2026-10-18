#ifndef KH_LOCAL_ACHIEVEMENTS_H
#define KH_LOCAL_ACHIEVEMENTS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KH_LOCAL_COUNT 16
typedef struct KhLocalDefinition {
    const char *title_es, *title_en, *description_es, *description_en;
} KhLocalDefinition;
extern const KhLocalDefinition gKhLocalDefinitions[KH_LOCAL_COUNT];

typedef struct KhLocalSnapshot {
    uint32_t flags, moogle_points;
    uint64_t learned, learned2, card_kinds;
    int level, max_hp, cp, floor, playing;
    uint16_t friends;
} KhLocalSnapshot;

typedef struct KhLocalState {
    uint32_t unlocked, previous;
    uint64_t when[KH_LOCAL_COUNT];
    int initialized;
} KhLocalState;

/* First valid observation establishes a baseline. Existing saves do not grant
 * achievements retroactively; awards require a transition seen in gameplay. */
uint32_t KhLocalObserve(KhLocalState* state, const KhLocalSnapshot* snapshot, uint64_t now);
int KhLocalLoad(KhLocalState* state, const char* path);
int KhLocalSave(const KhLocalState* state, const char* path);

#ifdef __cplusplus
}
#endif
#endif
