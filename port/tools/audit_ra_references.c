/* Offline evaluation audit: query the public set through the already connected
 * account, then export condition memory references. Never calls do_frame or
 * starts a game session; passwords/tokens and account unlocks are not exported.
 * Internal graph layout is tied to our pinned rcheevos revision.
 */
#include <stdio.h>
#include <SDL3/SDL.h>
#include "../achievements/ra_client.h"
#include "../third_party/rcheevos/src/rc_client_internal.h"
#include "../third_party/rcheevos/src/rcheevos/rc_internal.h"

static unsigned references;
static void Operand(FILE* out, unsigned id, unsigned group, unsigned condition,
                    const rc_operand_t* operand, unsigned depth) {
    const rc_memref_t* ref;
    if (!rc_operand_is_memref(operand) || depth > 32) return;
    ref = operand->value.memref;
    if (!ref) return;
    fprintf(out, "%u,%u,%u,%u,%u,%u,%u\n", id, group, condition, ref->address,
            (unsigned)operand->size, (unsigned)ref->value.memref_type, depth);
    ++references;
    if (ref->value.memref_type == RC_MEMREF_TYPE_MODIFIED_MEMREF) {
        const rc_modified_memref_t* modified = (const rc_modified_memref_t*)ref;
        Operand(out, id, group, condition, &modified->parent, depth + 1);
        Operand(out, id, group, condition, &modified->modifier, depth + 1);
    }
}
static void Groups(FILE* out, unsigned id, rc_condset_t* group, unsigned index) {
    for (; group; group = group->next, ++index) {
        rc_condition_t* condition; unsigned n = 0;
        for (condition = group->conditions; condition; condition = condition->next, ++n) {
            Operand(out, id, index, n, &condition->operand1, 0);
            Operand(out, id, index, n, &condition->operand2, 0);
        }
    }
}
int main(int argc, char** argv) {
    KhRa* ra; rc_client_t* client; rc_client_subset_info_t* subset;
    void* rom; size_t size; FILE* out; unsigned achievements = 0; Uint64 until;
    if (argc != 4 || !SDL_Init(0)) return 2;
    rom = SDL_LoadFile(argv[2], &size); if (!rom) return 2;
    ra = KhRaCreate(argv[1], NULL, NULL, NULL);
    if (!ra || !KhRaEnabled(ra) || !KhRaValidationMode()) return 2;
    KhRaSetRom(ra, rom, size); SDL_free(rom); client = KhRaClient(ra);
    until = SDL_GetTicks() + 30000;
    while (!rc_client_is_game_loaded(client) && SDL_GetTicks() < until) { KhRaPump(ra, 0); SDL_Delay(10); }
    if (!client->game) { puts("Catalog unavailable. Connect the account in the launcher first."); KhRaDestroy(ra); return 2; }
    out = fopen(argv[3], "w"); if (!out) { KhRaDestroy(ra); return 2; }
    fputs("achievement,group,condition,address,memsize,reference_type,depth\n", out);
    for (subset = client->game->subsets; subset; subset = subset->next) {
        unsigned i;
        for (i = 0; i < subset->public_.num_achievements; ++i) {
            rc_client_achievement_info_t* achievement = &subset->achievements[i];
            if (achievement->public_.category != RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE || !achievement->trigger) continue;
            ++achievements;
            Groups(out, achievement->public_.id, achievement->trigger->requirement, 0);
            Groups(out, achievement->public_.id, achievement->trigger->alternative, 1);
        }
    }
    fclose(out); KhRaDestroy(ra); SDL_Quit();
    printf("Audited %u core achievement graphs, %u references; no frames evaluated or awards submitted.\n", achievements, references);
    return achievements ? 0 : 1;
}
