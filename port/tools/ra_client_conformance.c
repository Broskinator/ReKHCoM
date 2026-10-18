#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "../achievements/ra_client.h"

static int reject_login, login_count, token_logins, game_requests, unlock_requests, unexpected;
static uint8_t memory[4];
static const char* catalog = "{\"Success\":true,\"GameId\":768,\"Title\":\"Synthetic integration fixture\",\"ConsoleId\":5,"
    "\"ImageIconUrl\":\"https://media.retroachievements.org/Images/1.png\",\"RichPresenceGameId\":768,"
    "\"RichPresencePatch\":\"\",\"Sets\":[{\"AchievementSetId\":1,\"GameId\":768,\"Title\":null,\"Type\":\"core\","
    "\"ImageIconUrl\":\"https://media.retroachievements.org/Images/1.png\",\"Achievements\":[{\"ID\":1,"
    "\"Title\":\"Fixture only\",\"Description\":\"Not a real account achievement\",\"Flags\":3,\"Points\":5,"
    "\"MemAddr\":\"0xH0001=1\",\"Author\":\"Fixture\",\"BadgeName\":\"00001\",\"Created\":1,\"Modified\":1}],\"Leaderboards\":[]}]}";

static void Transport(const rc_api_request_t* request, rc_client_server_callback_t callback, void* data) {
    rc_api_server_response_t response;
    const char* post = request->post_data ? request->post_data : "";
    assert(strncmp(request->url, "https://retroachievements.org/", 30) == 0);
    if (strstr(post, "r=login2")) {
        ++login_count;
        if (strstr(post, "&t=")) ++token_logins;
        response.body = reject_login ? "{\"Success\":false,\"Error\":\"Synthetic rejected credentials\"}" :
            "{\"Success\":true,\"User\":\"Fixture\",\"Token\":\"SyntheticOnlyToken\",\"Score\":10,\"SoftcoreScore\":5,\"Messages\":0}";
    } else if (strstr(post, "r=achievementsets")) { ++game_requests; response.body = catalog; }
    else if (strstr(post, "r=unlocks")) { ++unlock_requests; response.body = "{\"Success\":true,\"UserUnlocks\":[1],\"GameID\":768,\"HardcoreMode\":false}"; }
    else { ++unexpected; response.body = "{\"Success\":false,\"Error\":\"Unexpected request\"}"; }
    response.body_length = strlen(response.body); response.http_status_code = 200;
    callback(&response, data);
}

static uint32_t Read(uint32_t address, uint8_t* out, uint32_t size, void* userdata) {
    (void)userdata;
    if (address >= sizeof(memory) || size > sizeof(memory) - address) return 0;
    memcpy(out, memory + address, size); return size;
}

int main(int argc, char** argv) {
    KhRa* ra;
    FILE* file;
    unsigned char encrypted[4096];
    size_t size, i;
    assert(SDL_Init(0));
    remove("retroachievements.auth"); remove("retroachievements.ini");
    ra = KhRaCreate(".", Read, NULL, Transport); assert(ra);
    assert(!KhRaEnabled(ra) && !login_count);
    KhRaSetEnabled(ra, 1); reject_login = 1;
    KhRaLogin(ra, "Fixture", "InvalidSynthetic", 0);
    assert(KhRaError(ra) && !KhRaBusy(ra) && !rc_client_get_user_info(KhRaClient(ra)));
    reject_login = 0;
    KhRaLogin(ra, "Fixture", "SyntheticPassword", 1);
    assert(!KhRaError(ra) && rc_client_get_user_info(KhRaClient(ra)));
    file = fopen("retroachievements.auth", "rb"); assert(file);
    size = fread(encrypted, 1, sizeof(encrypted), file); fclose(file); assert(size > 256);
    for (i = 0; i + strlen("SyntheticOnlyToken") <= size; ++i)
        assert(memcmp(encrypted + i, "SyntheticOnlyToken", strlen("SyntheticOnlyToken")) != 0);
    if (argc > 1) {
        uint8_t* rom;
        file = fopen(argv[1], "rb"); assert(file);
        rom = (uint8_t*)malloc(32u << 20); assert(rom);
        assert(fread(rom, 1, 32u << 20, file) == (32u << 20)); fclose(file);
        KhRaSetRom(ra, rom, 32u << 20); free(rom);
        assert(strcmp(KhRaHash(ra), "d369f791d86a969778d2c3a5278d544c") == 0);
        assert(game_requests == 1 && unlock_requests == 1 && !KhRaBusy(ra));
        assert(rc_client_is_game_loaded(KhRaClient(ra)) && KhRaAccountUnlocked(ra, 1));
        KhRaPump(ra, 1); memory[1] = 1; KhRaPump(ra, 1);
        assert(KhRaTakeNotice(ra) != NULL);
        assert(KhRaTakeNotice(ra) == NULL);
    }
    assert(KhRaValidationMode() && !unexpected);
    assert(!rc_client_get_hardcore_enabled(KhRaClient(ra)));
    assert(rc_client_get_spectator_mode_enabled(KhRaClient(ra)));
    KhRaDestroy(ra);
    ra = KhRaCreate(".", Read, NULL, Transport); assert(ra);
    assert(token_logins == 1 && rc_client_get_user_info(KhRaClient(ra)));
    KhRaLogout(ra); assert(!rc_client_get_user_info(KhRaClient(ra)));
    KhRaDestroy(ra);
    ra = KhRaCreate(".", Read, NULL, Transport); assert(ra);
    assert(!rc_client_get_user_info(KhRaClient(ra)) && token_logins == 1);
    KhRaDestroy(ra); SDL_Quit();
    puts("PASS rcheevos integration: rejected login, encrypted token, reconnect, logout, no session/award submissions");
    return 0;
}
