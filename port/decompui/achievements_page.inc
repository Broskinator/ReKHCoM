/* Uses the launcher's white/black/red palette and the same data as the game. */
static KhRa* sRaUi;
static int sRaRomRegion = -1;
static bool sLocalAchievementsEnabled = true;
static char sRaUsername[128], sRaPassword[256], sAchievementSearch[128];
static bool sRaRemember = true;
static int sAchievementFilter;
static KhLocalState sLocalProgress;
static Uint64 sLocalReadAt;
static int sLocalReadRegion = -1;
static bool sAchievementPreview;
static Uint64 sAchievementPreviewStart;
static bool sRaAccountShot;

static void AchievementPreviewDraw() {
    if (!sAchievementPreview) return;
    unsigned elapsed = (unsigned)(SDL_GetTicks() - sAchievementPreviewStart);
    if (elapsed >= 4500) { sAchievementPreview = false; return; }
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    KhToastLayout box = KhAchievementToastLayout(elapsed, (int)screen.x, (int)screen.y, 220);
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImVec2 a(box.x, box.y), b(box.x + box.width, box.y + box.height);
    auto fade = [box](const ImVec4& color) { return ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, color.w * box.alpha)); };
    dl->AddRectFilled(ImVec2(a.x + box.scale, a.y + 2 * box.scale), ImVec2(b.x + box.scale, b.y + 2 * box.scale),
                      ImGui::GetColorU32(ImVec4(0, 0, 0, 0.2f * box.alpha)));
    dl->AddRectFilled(a, b, fade(kPanel)); dl->AddRect(a, b, fade(kBorder));
    dl->AddRectFilled(a, ImVec2(a.x + 3 * box.scale, b.y), fade(kAccent));
    float textX = a.x + 38 * box.scale;
    dl->AddRectFilled(ImVec2(a.x + 10 * box.scale, a.y + 16 * box.scale),
                      ImVec2(a.x + 31 * box.scale, a.y + 37 * box.scale), fade(kText));
    dl->AddRectFilled(ImVec2(a.x + 15 * box.scale, a.y + 21 * box.scale),
                      ImVec2(a.x + 26 * box.scale, a.y + 32 * box.scale), fade(kAccent));
    dl->AddText(sFontSmall, 6.0f * box.scale, ImVec2(textX, a.y + 5 * box.scale), fade(kMuted),
                T("VISTA PREVIA · LOGRO LOCAL", "PREVIEW · LOCAL ACHIEVEMENT"));
    dl->AddText(sFontBody, 8.0f * box.scale, ImVec2(textX, a.y + 18 * box.scale), fade(kText),
                T("Primeros recuerdos", "First memories"));
    dl->AddText(sFontSmall, 6.0f * box.scale, ImVec2(textX, a.y + 33 * box.scale), fade(kMuted),
                T("Alcanza el nivel 10.", "Reach level 10."));
}

static void LocalAchievementSettings(bool save) {
    fs::path path = sRoot / "achievements_local.ini";
    if (save) {
        std::ofstream file(path);
        file << "enabled=" << (sLocalAchievementsEnabled ? 1 : 0) << "\n";
        if (!file) Notice(T("No se pudo guardar la opción de logros.", "Could not save achievement settings."), true);
    } else {
        std::ifstream file(path);
        std::string line;
        if (std::getline(file, line)) sLocalAchievementsEnabled = line != "enabled=0";
    }
}

static bool AchievementMatches(const char* title, const char* description, bool unlocked) {
    if (sAchievementFilter == 1 && unlocked) return false;
    if (sAchievementFilter == 2 && !unlocked) return false;
    if (!sAchievementSearch[0]) return true;
    auto contains = [](const char* text) {
        return std::search(text, text + std::strlen(text), sAchievementSearch,
            sAchievementSearch + std::strlen(sAchievementSearch), [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            }) != text + std::strlen(text);
    };
    return contains(title) || contains(description);
}

static void AchievementRow(int id, const char* title, const char* description, bool unlocked,
                           const char* detail = nullptr) {
    ImGui::PushID(id);
    ImVec2 origin = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    ImGui::BeginGroup();
    ImGui::Dummy(ImVec2(Px(42), 0));
    ImGui::SameLine(0, Px(12));
    ImGui::BeginGroup();
    ImGui::TextColored(unlocked ? kAccentDim : kText, "%s", title);
    ImGui::PushFont(sFontSmall);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - Px(70));
    ImGui::TextColored(kMuted, "%s", description);
    if (detail && *detail) ImGui::TextColored(kMuted, "%s", detail);
    else ImGui::TextColored(unlocked ? kGood : kMuted, "%s", unlocked ? T("Desbloqueado", "Unlocked") : T("Pendiente", "Locked"));
    ImGui::PopTextWrapPos();
    ImGui::PopFont();
    ImGui::EndGroup();
    ImGui::EndGroup();
    GameSquare(ImGui::GetWindowDrawList(), ImVec2(origin.x + Px(20), origin.y + Px(21)), Px(28), unlocked);
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    ImGui::PopID();
}

static void AchievementFilters() {
    ImGui::SetNextItemWidth(Px(240));
    ImGui::InputTextWithHint("##achsearch", T("Buscar logro...", "Find achievement..."), sAchievementSearch, sizeof(sAchievementSearch));
    ImGui::SameLine();
    const char* filters[] = {T("Todos", "All"), T("Pendientes", "Locked"), T("Desbloqueados", "Unlocked")};
    ImGui::SetNextItemWidth(Px(180));
    ImGui::Combo("##achfilter", &sAchievementFilter, filters, 3);
    ImGui::Spacing();
}

static void PageLocalAchievements() {
    BeginCard("local-achievements");
    Eyebrow(T("RECUERDOS DE ESTA VERSIÓN", "MEMORIES OF THIS VERSION"));
    if (ImGui::Checkbox(T("Logros locales", "Local achievements"), &sLocalAchievementsEnabled)) LocalAchievementSettings(true);
    ImGui::SameLine();
    if (ImGui::Button(T("Probar aviso", "Preview notification"))) {
        sAchievementPreview = true; sAchievementPreviewStart = SDL_GetTicks();
    }
    ImGui::PushFont(sFontSmall);
    ImGui::TextWrapped("%s", T("16 retos del port para EU, US y JP. Se guardan por región en este equipo y se anuncian al jugar."
        " Los trucos, las revanchas y cargar un savestate pausan los logros hasta reiniciar el juego.",
        "16 port challenges for EU, US and JP. Progress is stored per region on this computer, with in-game notices."
        " Cheats, rematches and loading a save state pause achievements until the game is restarted."));
    ImGui::TextWrapped("%s", T("Las partidas existentes establecen el punto de partida: los retos se consiguen al avanzar, sin desbloqueos retroactivos.",
        "Existing saves establish your starting point: challenges unlock as you progress, without retroactive awards."));
    ImGui::PopFont();
    if (sLocalReadRegion != sSelected || SDL_GetTicks() - sLocalReadAt > 1000) {
        std::string path = (sRoot / (std::string("achievements_") + kVersions[sSelected].id + ".dat")).u8string();
        KhLocalLoad(&sLocalProgress, path.c_str());
        sLocalReadRegion = sSelected; sLocalReadAt = SDL_GetTicks();
    }
    int count = 0;
    for (int i = 0; i < KH_LOCAL_COUNT; ++i) if (sLocalProgress.unlocked & (1u << i)) ++count;
    char summary[120];
    std::snprintf(summary, sizeof(summary), T("%d / %d recuerdos · %s", "%d / %d memories · %s"), count, KH_LOCAL_COUNT, kVersions[sSelected].code);
    ImGui::Spacing();
    ImGui::ProgressBar((float)count / KH_LOCAL_COUNT, ImVec2(-1, Px(24)), summary);
    ImGui::Spacing();
    AchievementFilters();
    int shown = 0;
    bool table = ImGui::BeginTable("local-achievement-grid", ImGui::GetContentRegionAvail().x >= Px(850) ? 2 : 1,
                                  ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PadOuterX);
    for (int i = 0; i < KH_LOCAL_COUNT; ++i) {
        const KhLocalDefinition& def = gKhLocalDefinitions[i];
        const char *title = sSpanish ? def.title_es : def.title_en;
        const char *description = sSpanish ? def.description_es : def.description_en;
        bool unlocked = (sLocalProgress.unlocked & (1u << i)) != 0;
        if (!AchievementMatches(title, description, unlocked)) continue;
        if (table) ImGui::TableNextColumn();
        AchievementRow(i, title, description, unlocked);
        ++shown;
    }
    if (table) ImGui::EndTable();
    if (!shown) ImGui::TextDisabled("%s", T("No hay logros con ese filtro.", "No achievements match this filter."));
    ImGui::EndChild();
}

static void PageOfficialAchievements() {
    BeginCard("ra-account");
    Eyebrow("RETROACHIEVEMENTS");
    ImGui::TextWrapped("%s", T("Conexión experimental para consultar el catálogo y el progreso de tu cuenta."
        " Los desbloqueos oficiales y Hardcore todavía están desactivados.",
        "Experimental connection for browsing the catalog and your account progress."
        " Official unlock submissions and Hardcore are currently disabled."));
    bool enabled = KhRaEnabled(sRaUi);
    ImGui::BeginDisabled(KhRaBusy(sRaUi));
    if (ImGui::Checkbox(T("Conectar con RetroAchievements", "Connect to RetroAchievements"), &enabled)) KhRaSetEnabled(sRaUi, enabled);
    ImGui::EndDisabled();
    ImGui::PushFont(sFontSmall);
    ImGui::TextColored(KhRaError(sRaUi) ? kBad : kMuted, "%s", KhRaStatus(sRaUi));
    ImGui::PopFont();
    rc_client_t* client = KhRaClient(sRaUi);
    const rc_client_user_t* user = client ? rc_client_get_user_info(client) : nullptr;
    if (user) {
        ImGui::Spacing();
        ImGui::TextColored(kGood, "%s", user->display_name);
        ImGui::Text(T("Puntos: %u · Softcore: %u", "Points: %u · Softcore: %u"), user->score, user->score_softcore);
        ImGui::BeginDisabled(KhRaBusy(sRaUi));
        if (ImGui::Button(T("Cerrar sesión y olvidar token", "Sign out and forget token"))) KhRaLogout(sRaUi);
        ImGui::EndDisabled();
    } else if (enabled) {
        ImGui::BeginDisabled(KhRaBusy(sRaUi));
        ImGui::SetNextItemWidth(Px(320));
        ImGui::InputText(T("Usuario", "Username"), sRaUsername, sizeof(sRaUsername));
        ImGui::SetNextItemWidth(Px(320));
        ImGui::InputText(T("Contraseña", "Password"), sRaPassword, sizeof(sRaPassword), ImGuiInputTextFlags_Password);
        ImGui::Checkbox(T("Recordar esta cuenta en Windows", "Remember this account on Windows"), &sRaRemember);
        if (ImGui::Button(T("Iniciar sesión", "Sign in"))) {
            KhRaLogin(sRaUi, sRaUsername, sRaPassword, sRaRemember);
            SecureZeroMemory(sRaPassword, sizeof(sRaPassword));
        }
        ImGui::EndDisabled();
        ImGui::PushFont(sFontSmall);
        ImGui::TextWrapped("%s", T("La contraseña se envía por HTTPS a RetroAchievements y no se guarda."
            " Si recuerdas la cuenta, Windows cifra el token de sesión para tu usuario.",
            "The password is sent to RetroAchievements over HTTPS and is never stored."
            " Remembering your account stores a session token encrypted for your Windows user."));
        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::Spacing();
    BeginCard("ra-catalog");
    Eyebrow(T("CATÁLOGO OFICIAL", "OFFICIAL CATALOG"));
    ImGui::TextWrapped("%s", T("El set base reconoce la ROM europea. US y JP disponen de los retos locales de la otra pestaña.",
        "The base set recognizes the European ROM. US and JP have local challenges in the other tab."));
    ImGui::PushFont(sFontSmall);
    ImGui::TextWrapped("%s", T("Level 1 utiliza su propio parche de compatibilidad; el ejecutable nativo actual aún no lo valida.",
        "Level 1 uses its own compatibility patch; the current native executable does not yet validate it."));
    ImGui::PopFont();
    if (ImGui::Button(T("Ver ROMs compatibles y parche Level 1", "Supported ROMs and Level 1 patch")))
        SDL_OpenURL("https://retroachievements.org/game/768/hashes");
    if (client && rc_client_is_game_loaded(client)) {
        rc_client_achievement_list_t* list = rc_client_create_achievement_list(client,
            RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE, RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_LOCK_STATE);
        unsigned total = 0, earned = 0;
        if (list) for (uint32_t b = 0; b < list->num_buckets; ++b)
            for (uint32_t a = 0; a < list->buckets[b].num_achievements; ++a) {
                ++total;
                if (KhRaAccountUnlocked(sRaUi, list->buckets[b].achievements[a]->id)) ++earned;
            }
        char progress[100];
        std::snprintf(progress, sizeof(progress), "%u / %u", earned, total);
        ImGui::Spacing();
        if (KhRaAccountProgressAvailable(sRaUi)) ImGui::ProgressBar(total ? (float)earned / total : 0, ImVec2(-1, Px(24)), progress);
        else ImGui::TextDisabled("%s", T("Progreso de cuenta todavía no disponible.", "Account progress is not available yet."));
        AchievementFilters();
        if (list) {
            int shown = 0;
            for (uint32_t b = 0; b < list->num_buckets; ++b) for (uint32_t a = 0; a < list->buckets[b].num_achievements; ++a) {
                const rc_client_achievement_t* achievement = list->buckets[b].achievements[a];
                bool unlocked = KhRaAccountUnlocked(sRaUi, achievement->id) != 0;
                if (!AchievementMatches(achievement->title, achievement->description, unlocked)) continue;
                char detail[120];
                std::snprintf(detail, sizeof(detail), T("%u puntos · %s", "%u points · %s"), achievement->points,
                              !KhRaAccountProgressAvailable(sRaUi) ? T("Progreso sin consultar", "Progress not retrieved") :
                              unlocked ? T("Conseguido en tu cuenta", "Earned on your account") : T("Pendiente", "Locked"));
                AchievementRow((int)achievement->id, achievement->title, achievement->description, unlocked, detail);
                ++shown;
            }
            if (!shown) ImGui::TextDisabled("%s", T("No hay logros con ese filtro.", "No achievements match this filter."));
            rc_client_destroy_achievement_list(list);
        }
    }
    ImGui::EndChild();
}

static void PageAchievements() {
    const char* regions[] = {"EU · B8CP", "US · B8CE", "JP · B8CJ"};
    ImGui::SetNextItemWidth(Px(190));
    ImGui::Combo(T("Región", "Region"), &sSelected, regions, 3);
    if (sRaRomRegion != sSelected && !KhRaBusy(sRaUi)) {
        std::vector<unsigned char> rom;
        if (ReadFile(RomPath(sSelected), rom)) KhRaSetRom(sRaUi, rom.data(), rom.size());
        else KhRaSetRom(sRaUi, nullptr, 0);
        sRaRomRegion = sSelected;
    }
    if (ImGui::BeginTabBar("achievement-modes")) {
        if (ImGui::BeginTabItem(T("Logros locales", "Local achievements"))) { PageLocalAchievements(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem(T("Cuenta RetroAchievements", "RetroAchievements account"), nullptr,
            sRaAccountShot ? ImGuiTabItemFlags_SetSelected : 0)) { PageOfficialAchievements(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
}
