/* Public catalog metadata only. Account credentials and unlocks are never
   written here. The cached list stays browsable while offline/signed out. */
struct CachedAchievement {
    uint32_t id = 0, points = 0;
    std::string title, description, badge;
};
static std::vector<CachedAchievement> sAchievementCatalog;
static uint64_t sAchievementCatalogHash;
static fs::path AchievementCatalogPath() { return sRoot / "cache" / "achievements-768.dat"; }
static bool CatalogString(std::istream& in, std::string& value) {
    uint32_t size = 0; in.read((char*)&size, sizeof(size));
    if (!in || size > 16384) return false;
    value.resize(size); if (size) in.read(&value[0], size); return (bool)in;
}
static void CatalogString(std::ostream& out, const std::string& value) {
    uint32_t size = (uint32_t)value.size(); out.write((const char*)&size, sizeof(size)); out.write(value.data(), size);
}
static void ReadAchievementCatalog() {
    static bool read;
    if (read) return; read = true;
    std::ifstream in(AchievementCatalogPath(), std::ios::binary);
    char signature[8]; uint32_t count = 0;
    in.read(signature, 8); in.read((char*)&count, sizeof(count));
    if (!in || std::memcmp(signature, "KHRACAT1", 8) || count > 2000) return;
    std::vector<CachedAchievement> catalog;
    for (uint32_t i = 0; i < count; ++i) {
        CachedAchievement a;
        in.read((char*)&a.id, sizeof(a.id)); in.read((char*)&a.points, sizeof(a.points));
        if (!in || !CatalogString(in, a.title) || !CatalogString(in, a.description) || !CatalogString(in, a.badge) || a.badge.size() >= 8) return;
        catalog.push_back(a);
    }
    sAchievementCatalog = std::move(catalog);
}
static void RememberAchievementCatalog(rc_client_achievement_list_t* list) {
    if (!list) return;
    std::vector<CachedAchievement> catalog; uint64_t hash = 1469598103934665603ull;
    for (uint32_t b = 0; b < list->num_buckets; ++b) for (uint32_t i = 0; i < list->buckets[b].num_achievements; ++i) {
        const rc_client_achievement_t* a = list->buckets[b].achievements[i];
        catalog.push_back({a->id, a->points, a->title ? a->title : "", a->description ? a->description : "", a->badge_name});
    }
    std::sort(catalog.begin(), catalog.end(), [](const CachedAchievement& a, const CachedAchievement& b) { return a.id < b.id; });
    for (const auto& a : catalog) {
        hash ^= a.id; hash *= 1099511628211ull; hash ^= a.points; hash *= 1099511628211ull;
        for (const auto* text : {&a.title, &a.description, &a.badge}) for (unsigned char c : *text) { hash ^= c; hash *= 1099511628211ull; }
    }
    if (hash == sAchievementCatalogHash) return;
    sAchievementCatalogHash = hash; sAchievementCatalog = std::move(catalog);
    std::error_code error; fs::create_directories(AchievementCatalogPath().parent_path(), error);
    std::ofstream out(AchievementCatalogPath(), std::ios::binary);
    uint32_t count = (uint32_t)sAchievementCatalog.size(); out.write("KHRACAT1", 8); out.write((const char*)&count, sizeof(count));
    for (const auto& a : sAchievementCatalog) {
        out.write((const char*)&a.id, sizeof(a.id)); out.write((const char*)&a.points, sizeof(a.points));
        CatalogString(out, a.title); CatalogString(out, a.description); CatalogString(out, a.badge);
    }
}
static void RememberAchievementProgress() {
    static uint64_t previous;
    std::vector<uint32_t> ids;
    uint32_t known = KhRaAccountProgressAvailable(sRaUi) ? 1u : 0u;
    uint64_t hash = 1469598103934665603ull ^ known;
    for (const auto& a : sAchievementCatalog) if (known && KhRaAccountUnlocked(sRaUi, a.id)) {
        ids.push_back(a.id); hash ^= a.id; hash *= 1099511628211ull;
    }
    if (hash == previous) return;
    std::error_code error; fs::create_directories(AchievementCatalogPath().parent_path(), error);
    fs::path path = sRoot / "cache" / "achievement-unlocks.dat", temp = path; temp += ".tmp";
    std::ofstream out(temp, std::ios::binary);
    uint32_t count = (uint32_t)ids.size();
    out.write("KHRAPRG1", 8); out.write((const char*)&known, 4); out.write((const char*)&count, 4);
    if (count) out.write((const char*)ids.data(), count * 4);
    out.close();
    if (out && MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) previous = hash;
}
