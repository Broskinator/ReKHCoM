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
#include "rc_api_runtime.h"

static unsigned references;
static char sDefinitionsPath[1400];
static unsigned sDefinitions;
int __real_rc_api_process_fetch_game_sets_server_response(rc_api_fetch_game_sets_response_t*, const rc_api_server_response_t*);
static void JsonString(FILE* file, const char* value) {
    const unsigned char* p = (const unsigned char*)(value ? value : "");
    fputc('"', file);
    for (; *p; ++p) {
        if (*p == '"' || *p == '\\') { fputc('\\', file); fputc(*p, file); }
        else if (*p < 32) fprintf(file, "\\u%04x", *p);
        else fputc(*p, file);
    }
    fputc('"', file);
}
/* --wrap is used only by this diagnostic executable, never the game/client.
 * Serialize the selected public fields of the parsed response, not HTTP data. */
int __wrap_rc_api_process_fetch_game_sets_server_response(rc_api_fetch_game_sets_response_t* data, const rc_api_server_response_t* response) {
    int result = __real_rc_api_process_fetch_game_sets_server_response(data, response);
    if (result == RC_OK && data->response.succeeded && sDefinitionsPath[0]) {
        FILE* out = fopen(sDefinitionsPath, "w");
        if (out) {
            unsigned i, set; fputs("[\n", out); sDefinitions = 0;
            for (set = 0; set < data->num_sets; ++set) for (i = 0; i < data->sets[set].num_achievements; ++i) {
                const rc_api_achievement_definition_t* a = &data->sets[set].achievements[i];
                if (a->category != RC_ACHIEVEMENT_CATEGORY_CORE) continue;
                if (sDefinitions++) fputs(",\n", out);
                fprintf(out, "{\"id\":%u,\"definition\":", a->id); JsonString(out, a->definition); fputc('}', out);
            }
            fputs("\n]\n", out); fclose(out);
        }
    }
    return result;
}
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
    if (snprintf(sDefinitionsPath, sizeof(sDefinitionsPath), "%s.definitions.json", argv[3]) >= (int)sizeof(sDefinitionsPath)) return 2;
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
    printf("Audited %u core achievement graphs, %u references; %u public definitions exported, no frames evaluated or awards submitted.\n", achievements, references, sDefinitions);
    return achievements ? 0 : 1;
}
