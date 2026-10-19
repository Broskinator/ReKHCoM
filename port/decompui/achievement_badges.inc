/* Shared UI badge cache. Network stays off the GL thread; colour and grey
   textures are made once from the same image, so locked art stays recognisable. */
struct AchievementBadge {
    GLuint colour = 0, grey = 0;
    bool requested = false, received = false, cacheChecked = false;
    Uint64 retryAt = 0;
    std::vector<unsigned char> encoded;
    fs::path cache;
};
static std::map<std::string, AchievementBadge> sAchievementBadges;

static bool AchievementBadgePixels(AchievementBadge& badge, const unsigned char* rgba, int w, int h) {
    if (!rgba || w < 1 || h < 1 || w > 256 || h > 256) return false;
    std::vector<unsigned char> grey(rgba, rgba + w * h * 4);
    for (int i = 0; i < w * h; ++i) {
        unsigned v = (grey[i*4] * 54u + grey[i*4+1] * 183u + grey[i*4+2] * 19u + 128u) >> 8;
        grey[i*4] = grey[i*4+1] = grey[i*4+2] = (unsigned char)v;
    }
    for (int i = 0; i < 2; ++i) {
        GLuint* texture = i ? &badge.grey : &badge.colour;
        glGenTextures(1, texture); glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, i ? grey.data() : rgba);
    }
    return badge.colour && badge.grey;
}
static bool AchievementBadgeDecode(AchievementBadge& badge) {
    int w, h, channels;
    if (badge.encoded.empty() || badge.encoded.size() > (1u << 20) ||
        !stbi_info_from_memory(badge.encoded.data(), (int)badge.encoded.size(), &w, &h, &channels) ||
        w < 1 || h < 1 || w > 256 || h > 256) return false;
    unsigned char* rgba = stbi_load_from_memory(badge.encoded.data(), (int)badge.encoded.size(), &w, &h, &channels, 4);
    bool ok = AchievementBadgePixels(badge, rgba, w, h); stbi_image_free(rgba);
    return ok;
}
static void AchievementBadgeDownloaded(const uint8_t* bytes, size_t size, int status, void* userdata) {
    AchievementBadge& badge = *(AchievementBadge*)userdata;
    badge.received = true;
    if (status == 200 && size && size <= (1u << 20)) badge.encoded.assign(bytes, bytes + size);
    else badge.retryAt = SDL_GetTicks() + 60000;
}
static GLuint OfficialAchievementBadge(const rc_client_achievement_t* achievement, bool unlocked) {
    std::string name = achievement->badge_name;
    if (name.empty() || name.size() > 32 || name.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") != std::string::npos) return 0;
    AchievementBadge& badge = sAchievementBadges["ra_" + name];
    if (!badge.cacheChecked) {
        badge.cacheChecked = true;
        badge.cache = sRoot / "cache" / "achievement-badges" / (name + ".png");
        ReadFile(badge.cache, badge.encoded);
        if (!badge.encoded.empty()) { AchievementBadgeDecode(badge); badge.encoded.clear(); }
    }
    if (badge.received) {
        badge.received = false;
        if (!badge.encoded.empty() && AchievementBadgeDecode(badge)) {
            std::error_code error; fs::create_directories(badge.cache.parent_path(), error);
            std::ofstream file(badge.cache, std::ios::binary);
            if (file) file.write((const char*)badge.encoded.data(), (std::streamsize)badge.encoded.size());
        } else badge.retryAt = SDL_GetTicks() + 60000;
        badge.encoded.clear();
    }
    if (!badge.colour && ((!badge.requested && !badge.retryAt) || (badge.retryAt && SDL_GetTicks() >= badge.retryAt))) {
        char url[512];
        if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK) {
            badge.requested = KhRaFetchImage(sRaUi, url, AchievementBadgeDownloaded, &badge) != 0;
            badge.retryAt = badge.requested ? 0 : SDL_GetTicks() + 60000;
        }
    }
    if (badge.colour) badge.requested = true;
    return unlocked ? badge.colour : badge.grey;
}
static GLuint LocalAchievementBadge(int id, bool unlocked) {
    static const char* const kinds[KH_LOCAL_COUNT] = {"Kingdom Key","Three Wishes","Oathkeeper","Ultima Weapon",
        "Cure","Mega Potion","Bambi","Bambi","Cloud","Spellbinder","Lionheart","Oblivion","Soul Eater",
        "Crabclaw","Fairy Harp","One Winged Angel"};
    if (id < 0 || id >= KH_LOCAL_COUNT || !sState[sSelected].installed) return 0;
    AchievementBadge& badge = sAchievementBadges["local_" + std::to_string(sSelected) + "_" + std::to_string(id)];
    if (!badge.requested) {
        badge.requested = true;
        int card = -1;
        for (int i = 0; i < RekhcomCards::kCardCount; ++i)
            if (!std::strcmp(RekhcomCards::kKindNames[RekhcomCards::kCards[i].kind], kinds[id])) { card = i; break; }
        int sprite = card < 0 ? -1 : RekhcomCards::kCards[card].sprite[sSelected];
        static std::vector<unsigned char> rom;
        static int romRegion = -1;
        if (romRegion != sSelected) { rom.clear(); if (ReadFile(RomPath(sSelected), rom)) romRegion = sSelected; }
        if (sprite >= 0 && !rom.empty()) {
            unsigned tiles = RekhcomCards::kSprites[sprite].tiles[sSelected], palette = RekhcomCards::kSprites[sprite].palette[sSelected];
            if (tiles + 256 <= rom.size() && palette + 32 <= rom.size()) {
                unsigned char rgba[40*40*4] = {};
                for (int t = 0; t < 8; ++t) for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
                    unsigned char b = rom[tiles + t*32 + y*4 + x/2]; int c = x & 1 ? b >> 4 : b & 15;
                    if (!c) continue;
                    unsigned colour = rom[palette+c*2] | rom[palette+c*2+1] << 8;
                    unsigned char* p = rgba + ((4+(t/2)*8+y)*40+12+(t%2)*8+x)*4;
                    p[0]=(colour&31)*255/31;p[1]=((colour>>5)&31)*255/31;p[2]=((colour>>10)&31)*255/31;p[3]=255;
                }
                AchievementBadgePixels(badge, rgba, 40, 40);
            }
        }
    }
    return unlocked ? badge.colour : badge.grey;
}
static void AchievementBadgesDestroy() {
    for (auto& entry : sAchievementBadges) {
        if (entry.second.colour) glDeleteTextures(1, &entry.second.colour);
        if (entry.second.grey) glDeleteTextures(1, &entry.second.grey);
    }
    sAchievementBadges.clear();
}
