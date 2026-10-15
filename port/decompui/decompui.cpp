/*
 * decompui — the ReKHCoM launcher.
 *
 * Installs the user's cartridge dumps (any of the three versions, checked by
 * SHA-1), edits the game's settings file (kh_com.ini) and starts the game
 * executable built for the chosen version:
 *
 *   ReKHCoM.exe                 this launcher
 *   khcom_eu.exe / _us / _jp    the game, one per cartridge version
 *   roms/<code>.gba             installed dumps
 *   kh_com.ini, *.sav, *.log    settings, saves and logs (shared data folder)
 *
 * Dear ImGui on SDL3 + OpenGL. Its look follows recomp-ui's GBA theme.
 */
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <commdlg.h>
#include <shellapi.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl3.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

namespace fs = std::filesystem;

/* ---- strings (Spanish / English) ------------------------------------------- */

#ifndef REKHCOM_VERSION
#define REKHCOM_VERSION "v1.4"
#endif

static bool sSpanish;

static const char* T(const char* es, const char* en) {
    return sSpanish ? es : en;
}

/* ---- cartridge versions ---------------------------------------------------- */

struct Version {
    const char* id;
    const char* code;
    const char* sha1;
    const char* regionEs;
    const char* regionEn;
    const char* languages;
};

static const Version kVersions[] = {
    { "eu", "B8CP", "8db73586cdb11b3795907edebf43228dbcd3e6b2", "Europa", "Europe", "EN  FR  DE  ES  IT" },
    { "us", "B8CE", "10729bd884f8fdca7a310b6d606c52e46657aa48", "América", "USA", "EN" },
    { "jp", "B8CJ", "59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa", "Japón", "Japan", "JA" },
};
static const int kVersionCount = 3;
static const size_t kRomSize = 32u << 20;

struct VersionState {
    bool installed;
    fs::path exe;
};

static fs::path sRoot;
static VersionState sState[kVersionCount];
static int sSelected;
static std::string sNotice;
static bool sNoticeError;
static double sNoticeTime;

static void Notice(const std::string& text, bool error) {
    sNotice = text;
    sNoticeError = error;
    sNoticeTime = ImGui::GetTime();
}

static std::string Sha1Hex(const std::vector<unsigned char>& data) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    unsigned char digest[20] = {};
    std::string hex;

    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA1_ALGORITHM, nullptr, 0) != 0)
        return hex;
    if (BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0) == 0) {
        BCryptHashData(hash, (PUCHAR)data.data(), (ULONG)data.size(), 0);
        BCryptFinishHash(hash, digest, sizeof(digest), 0);
        BCryptDestroyHash(hash);
        for (unsigned char b : digest) {
            char two[3];
            std::snprintf(two, sizeof(two), "%02x", b);
            hex += two;
        }
    }
    BCryptCloseAlgorithmProvider(alg, 0);
    return hex;
}

static bool ReadFile(const fs::path& path, std::vector<unsigned char>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
        return false;
    f.seekg(0, std::ios::end);
    std::streamoff size = f.tellg();
    if (size <= 0 || size > (std::streamoff)(64u << 20))
        return false;
    out.resize((size_t)size);
    f.seekg(0);
    f.read((char*)out.data(), size);
    return (bool)f;
}

static fs::path RomPath(int v) {
    return sRoot / "roms" / (std::string(kVersions[v].code) + ".gba");
}

static void ScanInstall() {
    static const char* const devDirs[] = { "pc", "pc-us", "pc-jp" };

    for (int v = 0; v < kVersionCount; v++) {
        std::vector<unsigned char> rom;
        sState[v].installed = ReadFile(RomPath(v), rom) && Sha1Hex(rom) == kVersions[v].sha1;
        /* the game executables live in bin/, so the launcher is the only one at the root */
        sState[v].exe = sRoot / "bin" / (std::string("khcom_") + kVersions[v].id + ".exe");
        if (!fs::exists(sState[v].exe))
            sState[v].exe = sRoot / (std::string("khcom_") + kVersions[v].id + ".exe");
        if (!fs::exists(sState[v].exe)) {
            /* development tree: build/decompui next to build/pc, build/pc-us, ... */
            fs::path dev = sRoot.parent_path() / devDirs[v] / "khcom.exe";
            if (fs::exists(dev))
                sState[v].exe = dev;
        }
    }
}

/* Checks a dump and copies it into roms/. */
static void InstallRom(const fs::path& path) {
    std::vector<unsigned char> rom;

    if (!ReadFile(path, rom) || rom.size() != kRomSize) {
        Notice(T("Ese archivo no es un volcado de cartucho de 32 MiB.",
                 "That file is not a 32 MiB cartridge dump."), true);
        return;
    }
    for (int v = 0; v < kVersionCount; v++) {
        if (std::memcmp(rom.data() + 0xAC, kVersions[v].code, 4) != 0)
            continue;
        if (Sha1Hex(rom) != kVersions[v].sha1) {
            Notice(std::string(T("La ROM ", "The ")) + T(kVersions[v].regionEs, kVersions[v].regionEn) +
                       T(" no coincide con el volcado original (SHA-1 distinto).",
                         " ROM does not match the original dump (different SHA-1)."),
                   true);
            return;
        }
        std::error_code ec;
        fs::create_directories(sRoot / "roms", ec);
        fs::path tmp = RomPath(v);
        tmp += ".tmp";
        {
            std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
            out.write((const char*)rom.data(), (std::streamsize)rom.size());
            if (!out) {
                Notice(T("No se pudo escribir en la carpeta roms.", "Could not write to the roms folder."), true);
                return;
            }
        }
        fs::rename(tmp, RomPath(v), ec);
        if (ec) {
            Notice(T("No se pudo instalar la ROM.", "Could not install the ROM."), true);
            return;
        }
        sState[v].installed = true;
        sSelected = v;
        Notice(std::string(T("ROM instalada y verificada: ", "ROM installed and verified: ")) +
                   T(kVersions[v].regionEs, kVersions[v].regionEn),
               false);
        return;
    }
    Notice(T("No es una ROM de Kingdom Hearts: Chain of Memories.",
             "This is not a Kingdom Hearts: Chain of Memories ROM."), true);
}

static void PickRom(SDL_Window* window) {
    wchar_t file[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {};
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    ofn.lpstrFilter = L"GBA ROM (*.gba)\0*.gba;*.agb;*.bin\0*.*\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&ofn))
        InstallRom(fs::path(file));
}

/* ---- settings (kh_com.ini, shared with the game) ----------------------------- */

struct Setting {
    const char* key;
    int def;
};

/* Same keys, order and defaults as port/src/port_config.c. */
static const Setting kSettings[] = {
    { "config_version", 3 }, { "view_mode", 4 },   { "zoom", 1 },         { "scale_mode", 0 },
    { "filter", 0 },         { "fullscreen", 0 },  { "vsync", 1 },        { "hud_size", 1 },
    { "narrow_fit", 1 },     { "scanlines", 0 },   { "color_correction", 0 }, { "show_fps", 0 },
    { "window_scale", 4 },   { "fullscreen_width", 1920 }, { "fullscreen_height", 1080 },
    { "master_volume", 100 }, { "music_volume", 100 }, { "sfx_volume", 100 },
    { "dodge_button", 1 },   { "stock_button", 1 }, { "hd_text", 0 },
    { "card_hud_raise", 8 }, { "unstock_button", 1 }, { "save_bank", 1 }, { "hd2d", 0 }, { "smooth_audio", 0 },
    { "ps2_mode", 0 }, { "ps2_difficulty", 0 }, { "ps2_fast_text", 0 }, { "hd_hud", 0 }, { "aspect", 0 },
    { "kb_a", SDLK_X },         { "kb_a_2", SDLK_K },       { "pad_a", SDL_GAMEPAD_BUTTON_SOUTH },
    { "kb_b", SDLK_Z },         { "kb_b_2", SDLK_J },       { "pad_b", SDL_GAMEPAD_BUTTON_EAST },
    { "kb_l", SDLK_A },         { "kb_l_2", 0 },            { "pad_l", SDL_GAMEPAD_BUTTON_LEFT_SHOULDER },
    { "kb_r", SDLK_S },         { "kb_r_2", 0 },            { "pad_r", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER },
    { "kb_start", SDLK_RETURN }, { "kb_start_2", 0 },       { "pad_start", SDL_GAMEPAD_BUTTON_START },
    { "kb_select", SDLK_BACKSPACE }, { "kb_select_2", SDLK_RSHIFT }, { "pad_select", SDL_GAMEPAD_BUTTON_BACK },
    { "kb_up", SDLK_UP },       { "kb_up_2", SDLK_W },      { "pad_up", SDL_GAMEPAD_BUTTON_DPAD_UP },
    { "kb_down", SDLK_DOWN },   { "kb_down_2", 0 },         { "pad_down", SDL_GAMEPAD_BUTTON_DPAD_DOWN },
    { "kb_left", SDLK_LEFT },   { "kb_left_2", 0 },         { "pad_left", SDL_GAMEPAD_BUTTON_DPAD_LEFT },
    { "kb_right", SDLK_RIGHT }, { "kb_right_2", 0 },        { "pad_right", SDL_GAMEPAD_BUTTON_DPAD_RIGHT },
    { "kb_stock", SDLK_D },     { "kb_stock_2", 0 },        { "pad_stock", SDL_GAMEPAD_BUTTON_NORTH },
    { "kb_dodge", SDLK_C },     { "kb_dodge_2", 0 },        { "pad_dodge", SDL_GAMEPAD_BUTTON_WEST },
    { "kb_unstock", SDLK_V },   { "kb_unstock_2", 0 },      { "pad_unstock", SDL_GAMEPAD_BUTTON_RIGHT_STICK },
};

static std::map<std::string, int> sIni;

static int& Ini(const char* key) {
    return sIni[key];
}

static void LoadIni() {
    sIni.clear();
    for (const Setting& s : kSettings)
        sIni[s.key] = s.def;
    std::ifstream f(sRoot / "kh_com.ini");
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (line.empty() || line[0] == '#' || line[0] == ';' || eq == std::string::npos)
            continue;
        std::string key = line.substr(0, eq);
        while (!key.empty() && std::isspace((unsigned char)key.back()))
            key.pop_back();
        sIni[key] = std::atoi(line.c_str() + eq + 1);
    }
    /* Files from before the auto view switch to it once, as the game does. */
    if (sIni["config_version"] < 2) {
        sIni["view_mode"] = 4;
        sIni["config_version"] = 2;
    }
    /* ...and from before v1.0 to filling the height in GBA-size scenes. */
    if (sIni["config_version"] < 3) {
        sIni["narrow_fit"] = 1;
        sIni["config_version"] = 3;
    }
}

static void SaveIni() {
    std::ofstream f(sRoot / "kh_com.ini", std::ios::trunc);
    f << "# Kingdom Hearts: Chain of Memories (PC) settings\n";
    for (const Setting& s : kSettings)
        f << s.key << "=" << sIni[s.key] << "\n";
}

/* Language the European game starts in (the language screen's cursor):
   0 = the player chooses, then English, French, German, Italian, Spanish. */
static const char* const kGameLanguageIds[] = { "", "en", "fr", "de", "it", "es" };
static int sGameLanguage;
static int sUiLanguage; /* 1 = Spanish, 2 = English; 0 until chosen */

static void ApplyUiLanguage() {
    /* the first time, the system's language is chosen and kept */
    if (sUiLanguage != 1 && sUiLanguage != 2)
        sUiLanguage = PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_SPANISH ? 1 : 2;
    sSpanish = sUiLanguage == 1;
}

static void LoadLauncherState() {
    std::ifstream f(sRoot / "decompui.ini");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("version=", 0) == 0) {
            for (int v = 0; v < kVersionCount; v++)
                if (line.substr(8) == kVersions[v].id)
                    sSelected = v;
        } else if (line.rfind("game_language=", 0) == 0) {
            for (int i = 1; i < 6; i++)
                if (line.substr(14) == kGameLanguageIds[i])
                    sGameLanguage = i;
        } else if (line.rfind("ui_language=", 0) == 0) {
            sUiLanguage = std::clamp(std::atoi(line.c_str() + 12), 0, 2);
        }
    }
    ApplyUiLanguage();
}

static void SaveLauncherState() {
    std::ofstream f(sRoot / "decompui.ini", std::ios::trunc);
    f << "version=" << kVersions[sSelected].id << "\n";
    f << "game_language=" << kGameLanguageIds[sGameLanguage] << "\n";
    f << "ui_language=" << sUiLanguage << "\n";
}

/* ---- saves -------------------------------------------------------------------- */

/* The selected save bank's file: bank 1 is the version's .sav, bank n
   <name>_bank<n>.sav (port_memory.c). */
static fs::path SavePath(int v) {
    static const char* const names[] = { "kh_com", "kh_com_us", "kh_com_jp" };
    int bank = std::clamp(Ini("save_bank"), 1, 5);
    return sRoot / (std::string(names[v]) + (bank > 1 ? "_bank" + std::to_string(bank) : "") + ".sav");
}

/* Copies the version's save to backups/<name>-<date>-<time>.sav. */
static void BackupSave(int v) {
    std::error_code ec;
    SYSTEMTIME t;
    char stamp[32];

    GetLocalTime(&t);
    std::snprintf(stamp, sizeof(stamp), "-%04d%02d%02d-%02d%02d%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute,
                  t.wSecond);
    fs::path dir = sRoot / "backups";
    fs::create_directories(dir, ec);
    fs::path dst = dir / (SavePath(v).stem().string() + stamp + ".sav");
    if (fs::copy_file(SavePath(v), dst, fs::copy_options::overwrite_existing, ec))
        Notice(std::string(T("Copia guardada en backups/", "Backup saved to backups/")) + dst.filename().string(), false);
    else
        Notice(T("No se pudo copiar la partida.", "Could not copy the save."), true);
}

/* ---- importing a save from an emulator or a backup device ------------------- */

/* The game's SRAM is 64 KB of blocks, each starting with a 24-byte signature
   ("KHCOM_BACKUP_VER00000013" in Europe, "...12" in the USA and Japan), a
   ones'-complement checksum and the data; the layout is the same in all three
   versions. Any container (raw .sav/.srm, GameShark, Action Replay, ...) is
   read by finding the first signature, which the cartridge keeps at 0x10. */
static const char kSaveSig[] = "KHCOM_BACKUP_VER000000";

static uint16_t SaveSum(const unsigned char* p, size_t size) {
    uint32_t sum = 0;
    size_t i;
    for (i = 0; i + 1 < size; i += 2)
        sum += (uint32_t)(p[i] | (p[i + 1] << 8));
    if (size & 1)
        sum += p[size - 1];
    sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~(sum + (sum >> 16));
}

/* Returns the number of blocks converted, -1 when the file holds no save. */
static int ConvertGbaSave(const std::vector<unsigned char>& in, int v, std::vector<unsigned char>& out,
                          int& fromVersion) {
    static const size_t sizes[] = { 0x40, 0xF14, 0x418 };
    const size_t sigLen = sizeof(kSaveSig) - 1;
    size_t at = std::string::npos, o;
    int blocks = 0;

    for (o = 0; o + sigLen + 2 <= in.size(); o++)
        if (std::memcmp(in.data() + o, kSaveSig, sigLen) == 0) {
            at = o;
            break;
        }
    if (at == std::string::npos || at < 0x10)
        return -1;
    out.assign(0x10000, 0xFF);
    std::memcpy(out.data(), in.data() + at - 0x10, std::min<size_t>(0x10000, in.size() - (at - 0x10)));
    fromVersion = in[at + sigLen + 1] == '3' ? 0 : 1; /* 1: USA or Japan, the same signature */

    for (o = 0; o + 0x40 <= out.size(); o++) {
        size_t k;
        if (std::memcmp(out.data() + o, kSaveSig, sigLen) != 0)
            continue;
        for (k = 0; k < 3; k++) {
            size_t size = sizes[k];
            if (o + size > out.size() || SaveSum(out.data() + o, size) != 0)
                continue;
            out[o + sigLen] = '1';
            out[o + sigLen + 1] = v == 0 ? '3' : '2';
            /* the header's menu language: the USA and Japan have only one */
            if (size == 0x40 && v != 0)
                out[o + 0x1C] = out[o + 0x1D] = 0;
            out[o + 0x18] = out[o + 0x19] = 0;
            {
                uint16_t c = SaveSum(out.data() + o, size);
                out[o + 0x18] = (unsigned char)(c & 0xFF);
                out[o + 0x19] = (unsigned char)(c >> 8);
            }
            blocks++;
            o += size - 1;
            break;
        }
    }
    return blocks;
}

static void ImportSave(SDL_Window* window, int v) {
    wchar_t file[MAX_PATH] = L"";
    OPENFILENAMEW ofn = {};
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    std::vector<unsigned char> in, out;
    std::error_code ec;
    int from = 0, blocks;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    ofn.lpstrFilter = L"GBA save (*.sav;*.srm;*.sa1;*.sps;*.xps;*.gsv;*.dsv;*.bin)\0"
                      L"*.sav;*.srm;*.sa1;*.sps;*.xps;*.gsv;*.dsv;*.bin\0*.*\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn))
        return;
    if (!ReadFile(fs::path(file), in) || in.size() > (4u << 20)) {
        Notice(T("No se pudo leer el archivo.", "Could not read the file."), true);
        return;
    }
    blocks = ConvertGbaSave(in, v, out, from);
    if (blocks <= 0) {
        Notice(T("El archivo no contiene una partida de Chain of Memories.",
                 "The file holds no Chain of Memories save."), true);
        return;
    }
    if (fs::exists(SavePath(v), ec))
        BackupSave(v);
    {
        std::ofstream f(SavePath(v), std::ios::binary | std::ios::trunc);
        f.write((const char*)out.data(), (std::streamsize)out.size());
        if (!f) {
            Notice(T("No se pudo escribir la partida.", "Could not write the save."), true);
            return;
        }
    }
    {
        char msg[200];
        bool cross = (from == 0) != (v == 0);
        std::snprintf(msg, sizeof(msg), "%s %s (%d %s)%s",
                      T("Partida importada en", "Save imported into"), SavePath(v).filename().string().c_str(),
                      blocks, T("bloques", "blocks"),
                      cross ? T(", convertida de otra regi\xC3\xB3n", ", converted from another region") : "");
        Notice(msg, false);
    }
}

/* ---- theme ------------------------------------------------------------------- */

static float sScale = 1.0f;

static float Px(float v) {
    return v * sScale;
}

static ImU32 Col(float r, float g, float b, float a = 1.0f) {
    return ImGui::GetColorU32(ImVec4(r, g, b, a));
}

/* recomp-ui's "Game Boy Advance" palette. */
/* The game's look (its debug menu, which the in-game options copy): white,
   the grey of its bars, near-black text and the red of its cursor square. */
static const ImVec4 kInk(1.0f, 1.0f, 1.0f, 1.0f);
static const ImVec4 kInkLift(0.965f, 0.957f, 0.957f, 1.0f);
static const ImVec4 kPanel(0.988f, 0.984f, 0.984f, 1.0f);
static const ImVec4 kPanelHover(0.925f, 0.910f, 0.910f, 1.0f);
static const ImVec4 kControl(0.910f, 0.894f, 0.894f, 1.0f);
static const ImVec4 kControlHover(0.835f, 0.812f, 0.812f, 1.0f);
static const ImVec4 kBorder(0.710f, 0.678f, 0.678f, 1.0f);
static const ImVec4 kAccent(1.0f, 0.063f, 0.0f, 1.0f);
static const ImVec4 kAccentDim(0.612f, 0.063f, 0.0f, 1.0f);
static const ImVec4 kText(0.031f, 0.031f, 0.031f, 1.0f);
static const ImVec4 kMuted(0.420f, 0.396f, 0.396f, 1.0f);
static const ImVec4 kGood(0.090f, 0.520f, 0.220f, 1.0f);
static const ImVec4 kWarn(0.780f, 0.420f, 0.0f, 1.0f);
static const ImVec4 kBad(0.800f, 0.063f, 0.0f, 1.0f);
static const ImVec4 kFocus(0.031f, 0.031f, 0.031f, 1.0f);

/* The debug menu's cursor square: black with red inside (on), or grey. */
static void GameSquare(ImDrawList* dl, ImVec2 c, float size, bool on) {
    float h = size * 0.5f, in = size * 0.25f, edge = size * 0.08f;
    if (!on) {
        dl->AddRectFilled(ImVec2(c.x - h, c.y - h), ImVec2(c.x + h, c.y + h), ImGui::GetColorU32(kBorder));
        return;
    }
    dl->AddRectFilled(ImVec2(c.x - h, c.y - h), ImVec2(c.x + h, c.y + h), ImGui::GetColorU32(kText));
    dl->AddRectFilled(ImVec2(c.x - in, c.y - in), ImVec2(c.x + in, c.y + in), ImGui::GetColorU32(kAccent));
    dl->AddRectFilled(ImVec2(c.x - in, c.y - in), ImVec2(c.x - in + edge, c.y + in), ImGui::GetColorU32(kAccentDim));
    dl->AddRectFilled(ImVec2(c.x - in, c.y - in), ImVec2(c.x + in, c.y - in + edge), ImGui::GetColorU32(kAccentDim));
}

static ImFont* sFontBody;
static ImFont* sFontTitle;
static ImFont* sFontSmall;

static void ApplyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s = ImGuiStyle();
    s.WindowPadding = ImVec2(Px(24), Px(20));
    s.FramePadding = ImVec2(Px(14), Px(9));
    s.ItemSpacing = ImVec2(Px(10), Px(10));
    s.ChildRounding = 0;
    s.FrameRounding = 0;
    s.GrabRounding = 0;
    s.ScrollbarRounding = 0;
    s.ChildBorderSize = 1.0f;
    s.FrameBorderSize = 1.0f;
    s.ScrollbarSize = Px(12);
    s.GrabMinSize = Px(14);
    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ChildBg] = kPanel;
    c[ImGuiCol_Border] = kBorder;
    c[ImGuiCol_Text] = kText;
    c[ImGuiCol_TextDisabled] = kMuted;
    c[ImGuiCol_FrameBg] = kControl;
    c[ImGuiCol_FrameBgHovered] = kControlHover;
    c[ImGuiCol_FrameBgActive] = kControlHover;
    c[ImGuiCol_Button] = kControl;
    c[ImGuiCol_ButtonHovered] = kControlHover;
    c[ImGuiCol_ButtonActive] = kAccentDim;
    c[ImGuiCol_SliderGrab] = kAccent;
    c[ImGuiCol_SliderGrabActive] = kAccent;
    c[ImGuiCol_CheckMark] = kAccent;
    c[ImGuiCol_Header] = kControl;
    c[ImGuiCol_HeaderHovered] = kControlHover;
    c[ImGuiCol_HeaderActive] = kAccentDim;
    c[ImGuiCol_NavCursor] = kFocus;
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = kBorder;
    c[ImGuiCol_TableBorderLight] = kBorder;
    c[ImGuiCol_TableBorderStrong] = kBorder;
    c[ImGuiCol_TableRowBgAlt] = ImVec4(0, 0, 0, 0.025f);
    c[ImGuiCol_PopupBg] = kPanel;
}

static void LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    wchar_t windir[MAX_PATH];
    GetWindowsDirectoryW(windir, MAX_PATH);
    fs::path fonts = fs::path(windir) / "Fonts";
    std::string regular = (fonts / "segoeui.ttf").string();
    std::string semibold = (fonts / "seguisb.ttf").string();
    static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x2190, 0x2193, 0x25B6, 0x25C0, 0 };
    ImFontConfig cfg;
    cfg.OversampleH = 2;

    io.Fonts->Clear();
    if (fs::exists(regular)) {
        sFontBody = io.Fonts->AddFontFromFileTTF(regular.c_str(), Px(18), &cfg, ranges);
        sFontSmall = io.Fonts->AddFontFromFileTTF(regular.c_str(), Px(14), &cfg, ranges);
        sFontTitle = io.Fonts->AddFontFromFileTTF(fs::exists(semibold) ? semibold.c_str() : regular.c_str(),
                                                  Px(34), &cfg, ranges);
    }
    if (sFontBody == nullptr) {
        ImFontConfig def;
        def.SizePixels = Px(16);
        sFontBody = sFontSmall = sFontTitle = io.Fonts->AddFontDefault(&def);
    }
}

/* ---- widgets ----------------------------------------------------------------- */

static void Background(ImVec2 size) {
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    ImU32 ink = ImGui::GetColorU32(kInk), lift = ImGui::GetColorU32(kInkLift);
    float mid = size.y * 0.42f;
    bg->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(size.x, mid), ink, ink, lift, lift);
    bg->AddRectFilledMultiColor(ImVec2(0, mid), size, lift, lift, ink, ink);
}

/* Uppercase section heading with an accent tick. */
static void Eyebrow(const char* text) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::PushFont(sFontSmall);
    float h = ImGui::GetTextLineHeight();
    GameSquare(dl, ImVec2(p.x + h * 0.4f, p.y + h * 0.5f), h * 0.8f, true);
    float x = p.x + h * 1.1f;
    char one[2] = { 0, 0 };
    for (const char* q = text; *q; q++) {
        one[0] = *q;
        dl->AddText(ImVec2(x, p.y), ImGui::GetColorU32(kText), one);
        x += ImGui::CalcTextSize(one).x + Px(2.2f);
    }
    ImGui::Dummy(ImVec2(x - p.x, h));
    ImGui::PopFont();
    ImGui::Spacing();
}

static bool BeginCard(const char* id, ImVec2 size = ImVec2(0, 0)) {
    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (size.y == 0)
        flags |= ImGuiChildFlags_AutoResizeY;
    return ImGui::BeginChild(id, size, flags);
}

/* The primary action: glow, accent gradient and a play triangle. */
static bool PlayButton(const char* label, ImVec2 size, bool enabled) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    if (!enabled)
        ImGui::BeginDisabled();
    bool clicked = ImGui::InvisibleButton("##play", size, ImGuiButtonFlags_EnableNav);
    bool hover = enabled && ImGui::IsItemHovered();
    bool focus = ImGui::IsItemFocused();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mx(p.x + size.x, p.y + size.y);
    float r = Px(8);
    float a = enabled ? 1.0f : 0.35f;

    /* a debug-menu row: black, the red cursor square, a grey shadow */
    dl->AddRectFilled(ImVec2(p.x + Px(4), p.y + Px(4)), ImVec2(mx.x + Px(4), mx.y + Px(4)),
                      Col(kBorder.x, kBorder.y, kBorder.z, a));
    dl->AddRectFilled(p, mx, hover ? Col(0.18f, 0.17f, 0.17f, a) : Col(kText.x, kText.y, kText.z, a));
    (void)r;
    if (focus)
        dl->AddRect(ImVec2(p.x - Px(3), p.y - Px(3)), ImVec2(mx.x + Px(3), mx.y + Px(3)),
                    ImGui::GetColorU32(kAccent), 0, 0, Px(2.5f));
    ImGui::PushFont(sFontTitle);
    float th = ImGui::GetTextLineHeight(), tw = ImGui::CalcTextSize(label).x;
    float sq = Px(22), gap = Px(16);
    float cx = p.x + (size.x - sq - gap - tw) * 0.5f, cy = p.y + size.y * 0.5f;
    if (enabled) {
        dl->AddRectFilled(ImVec2(cx, cy - sq * 0.5f), ImVec2(cx + sq, cy + sq * 0.5f), Col(1, 1, 1, a));
        GameSquare(dl, ImVec2(cx + sq * 0.5f, cy), sq * 0.8f, true);
    } else {
        GameSquare(dl, ImVec2(cx + sq * 0.5f, cy), sq * 0.8f, false);
    }
    dl->AddText(ImVec2(cx + sq + gap, cy - th * 0.5f), Col(1, 1, 1, a), label);
    ImGui::PopFont();
    if (!enabled)
        ImGui::EndDisabled();
    return clicked && enabled;
}

static void StatusDot(const ImVec4& color) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = ImGui::GetTextLineHeight(), r = Px(5);
    dl->AddCircleFilled(ImVec2(p.x + r, p.y + h * 0.5f), r, ImGui::GetColorU32(color));
    ImGui::Dummy(ImVec2(r * 2, h));
    ImGui::SameLine(0, Px(8));
}

/* A settings row: label on the left, "<  value  >" on the right. */
static bool CycleRow(const char* label, const char* hint, int& value, int count, const char* const* names) {
    bool changed = false;
    float w = ImGui::GetContentRegionAvail().x;
    float ctrl = std::min(Px(300), w * 0.5f);

    ImGui::PushID(label);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    if (hint != nullptr && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", hint);
    ImGui::SameLine(w - ctrl);
    float btn = ImGui::GetFrameHeight();
    if (ImGui::Button("<", ImVec2(btn, 0))) {
        value = (value + count - 1) % count;
        changed = true;
    }
    ImGui::SameLine(0, Px(6));
    ImVec2 p = ImGui::GetCursorScreenPos();
    float mid = ctrl - btn * 2 - Px(12);
    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + mid, p.y + btn), ImGui::GetColorU32(kControl), Px(6));
    const char* text = names[std::clamp(value, 0, count - 1)];
    float tw = ImGui::CalcTextSize(text).x;
    ImGui::GetWindowDrawList()->AddText(ImVec2(p.x + (mid - tw) * 0.5f, p.y + (btn - ImGui::GetTextLineHeight()) * 0.5f),
                                        ImGui::GetColorU32(kText), text);
    ImGui::Dummy(ImVec2(mid, btn));
    ImGui::SameLine(0, Px(6));
    if (ImGui::Button(">", ImVec2(btn, 0))) {
        value = (value + 1) % count;
        changed = true;
    }
    ImGui::PopID();
    return changed;
}

static bool ToggleRow(const char* label, const char* description, int& value) {
    bool changed = false;
    float w = ImGui::GetContentRegionAvail().x;
    float tw = Px(52), th = Px(28);

    ImGui::PushID(label);
    ImGui::BeginGroup();
    ImGui::TextUnformatted(label);
    if (description != nullptr) {
        ImGui::PushFont(sFontSmall);
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + w - tw - Px(24));
        ImGui::TextWrapped("%s", description);
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::EndGroup();
    float rowH = ImGui::GetItemRectSize().y;
    ImGui::SameLine(w - tw);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (rowH - th) * 0.5f);
    ImVec2 p = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("##toggle", ImVec2(tw, th), ImGuiButtonFlags_EnableNav)) {
        value = !value;
        changed = true;
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    /* on: the debug menu's black square with the red one inside; off: grey */
    GameSquare(dl, ImVec2(p.x + tw - th * 0.5f, p.y + th * 0.5f), th * 0.8f, value);
    ImGui::PushFont(sFontSmall);
    const char* state = value ? T("S\xC3\xAD", "On") : T("No", "Off");
    ImVec2 ss = ImGui::CalcTextSize(state);
    dl->AddText(ImVec2(p.x + tw - th - Px(6) - ss.x, p.y + (th - ss.y) * 0.5f),
                ImGui::GetColorU32(value ? kText : kMuted), state);
    ImGui::PopFont();
    if (ImGui::IsItemFocused())
        dl->AddRect(ImVec2(p.x - Px(3), p.y - Px(3)), ImVec2(p.x + tw + Px(3), p.y + th + Px(3)),
                    ImGui::GetColorU32(kFocus), 0, 0, Px(2));
    ImGui::PopID();
    return changed;
}

static bool SliderRow(const char* label, int& value, int lo, int hi, const char* format) {
    float w = ImGui::GetContentRegionAvail().x;
    float ctrl = std::min(Px(300), w * 0.5f);

    ImGui::PushID(label);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine(w - ctrl);
    ImGui::SetNextItemWidth(ctrl);
    bool changed = ImGui::SliderInt("##v", &value, lo, hi, format);
    ImGui::PopID();
    return changed;
}

/* ---- pages ------------------------------------------------------------------- */

enum Page { PAGE_HOME, PAGE_GRAPHICS, PAGE_SOUND, PAGE_EXTRAS, PAGE_CONTROLS, PAGE_COUNT };

static int sPage;
static GLuint sBoxArt;
static int sBoxW, sBoxH;
static bool sLaunch;

static GLuint sLogo;
static int sLogoW, sLogoH;

static bool LoadTexture(const fs::path& path, GLuint* tex, int* w, int* h) {
    std::vector<unsigned char> file;
    if (!ReadFile(path, file))
        return false;
    int n;
    unsigned char* rgba = stbi_load_from_memory(file.data(), (int)file.size(), w, h, &n, 4);
    if (rgba == nullptr)
        return false;
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *w, *h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    stbi_image_free(rgba);
    return true;
}

static void LoadBoxArt() {
    fs::path path = sRoot / "assets" / "boxart.png";
    if (!fs::exists(path))
        path = sRoot / "assets" / "boxart.jpg";
    LoadTexture(path, &sBoxArt, &sBoxW, &sBoxH);
    LoadTexture(sRoot / "assets" / "logo.png", &sLogo, &sLogoW, &sLogoH);
}

/* Box art, or a typographic cover when assets/boxart.png is missing. */
static void DrawCover(float w, float h) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (sBoxArt != 0) {
        float s = std::min(w / sBoxW, h / sBoxH);
        float iw = sBoxW * s, ih = sBoxH * s;
        ImGui::SetCursorScreenPos(ImVec2(p.x + (w - iw) * 0.5f, p.y + (h - ih) * 0.5f));
        ImGui::Image((ImTextureID)(intptr_t)sBoxArt, ImVec2(iw, ih));
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + h));
        ImGui::Dummy(ImVec2(w, 0));
        return;
    }
    ImVec2 mx(p.x + w, p.y + h);
    dl->AddRectFilledMultiColor(p, mx, Col(0.16f, 0.13f, 0.36f), Col(0.16f, 0.13f, 0.36f), Col(0.05f, 0.04f, 0.11f),
                                Col(0.05f, 0.04f, 0.11f));
    dl->AddRect(p, mx, ImGui::GetColorU32(kBorder), Px(10), 0, 1.0f);
    /* a card outline, the game's motif */
    float cw = w * 0.36f, ch = cw * 1.45f;
    ImVec2 c0(p.x + (w - cw) * 0.5f, p.y + h * 0.16f);
    dl->AddRectFilled(c0, ImVec2(c0.x + cw, c0.y + ch), Col(kAccent.x, kAccent.y, kAccent.z, 0.18f), Px(8));
    dl->AddRect(c0, ImVec2(c0.x + cw, c0.y + ch), Col(kAccent.x, kAccent.y, kAccent.z, 0.8f), Px(8), 0, Px(2));
    ImGui::PushFont(sFontTitle);
    const char* t1 = "KINGDOM HEARTS";
    float t1w = ImGui::CalcTextSize(t1).x;
    float scale = std::min(1.0f, (w - Px(24)) / t1w);
    ImGui::PopFont();
    dl->AddText(sFontTitle, sFontTitle->FontSize * scale, ImVec2(p.x + (w - t1w * scale) * 0.5f, c0.y + ch + Px(18)),
                ImGui::GetColorU32(kText), t1);
    const char* t2 = "Chain of Memories";
    float t2w = sFontBody->CalcTextSizeA(sFontBody->FontSize, FLT_MAX, 0, t2).x;
    dl->AddText(sFontBody, sFontBody->FontSize, ImVec2(p.x + (w - t2w) * 0.5f, c0.y + ch + Px(18) + Px(40) * scale),
                ImGui::GetColorU32(kMuted), t2);
    ImGui::Dummy(ImVec2(w, h));
}

static void PageHome(SDL_Window* window) {
    float avail = ImGui::GetContentRegionAvail().x;
    float coverW = std::clamp(avail * 0.34f, Px(240), Px(360));

    BeginCard("cover", ImVec2(coverW + Px(48), 0));
    Eyebrow(T("JUEGO", "GAME"));
    DrawCover(coverW, coverW * 1.25f);
    ImGui::EndChild();
    ImGui::SameLine(0, Px(20));

    ImGui::BeginGroup();
    BeginCard("versions");
    Eyebrow(T("VERSIONES", "VERSIONS"));
    for (int v = 0; v < kVersionCount; v++) {
        const Version& ver = kVersions[v];
        ImGui::PushID(v);
        bool selected = sSelected == v;
        ImVec2 rowMin = ImGui::GetCursorScreenPos();
        float rowW = ImGui::GetContentRegionAvail().x;
        float rowH = Px(64);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (ImGui::InvisibleButton("##row", ImVec2(rowW, rowH), ImGuiButtonFlags_EnableNav) && sState[v].installed) {
            sSelected = v;
            SaveLauncherState();
        }
        bool hover = ImGui::IsItemHovered();
        dl->AddRectFilled(rowMin, ImVec2(rowMin.x + rowW, rowMin.y + rowH),
                          ImGui::GetColorU32(selected ? kPanelHover : (hover ? kControl : kPanel)));
        dl->AddRectFilled(ImVec2(rowMin.x, rowMin.y + rowH - Px(3)), ImVec2(rowMin.x + rowW, rowMin.y + rowH),
                          ImGui::GetColorU32(selected ? kText : kBorder));
        if (ImGui::IsItemFocused())
            dl->AddRect(ImVec2(rowMin.x - Px(2), rowMin.y - Px(2)), ImVec2(rowMin.x + rowW + Px(2), rowMin.y + rowH + Px(2)),
                        ImGui::GetColorU32(kFocus), 0, 0, Px(2));
        /* the debug menu's square, as a radio */
        GameSquare(dl, ImVec2(rowMin.x + Px(22), rowMin.y + rowH * 0.5f), Px(16), selected);
        /* region, code and languages */
        float tx = rowMin.x + Px(44);
        dl->AddText(sFontBody, sFontBody->FontSize, ImVec2(tx, rowMin.y + Px(10)), ImGui::GetColorU32(kText),
                    T(ver.regionEs, ver.regionEn));
        char sub[64];
        std::snprintf(sub, sizeof(sub), "%s   %s", ver.code, ver.languages);
        dl->AddText(sFontSmall, sFontSmall->FontSize, ImVec2(tx, rowMin.y + Px(36)), ImGui::GetColorU32(kMuted), sub);
        /* status */
        const char* status;
        ImVec4 color;
        if (!sState[v].installed) {
            status = T("Sin ROM", "No ROM");
            color = kMuted;
        } else if (!fs::exists(sState[v].exe)) {
            status = T("Falta el ejecutable", "Executable missing");
            color = kWarn;
        } else {
            status = T("Verificada", "Verified");
            color = kGood;
        }
        float sw = sFontSmall->CalcTextSizeA(sFontSmall->FontSize, FLT_MAX, 0, status).x;
        ImVec2 sp(rowMin.x + rowW - sw - Px(18), rowMin.y + (rowH - sFontSmall->FontSize) * 0.5f);
        dl->AddCircleFilled(ImVec2(sp.x - Px(12), sp.y + sFontSmall->FontSize * 0.5f), Px(4.5f),
                            ImGui::GetColorU32(color));
        dl->AddText(sFontSmall, sFontSmall->FontSize, sp, ImGui::GetColorU32(color), status);
        ImGui::PopID();
    }
    ImGui::Spacing();
    if (ImGui::Button(T("Instalar ROM...", "Install ROM..."), ImVec2(-FLT_MIN, 0)))
        PickRom(window);
    ImGui::PushFont(sFontSmall);
    ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
    ImGui::TextWrapped("%s", T("Elige tu volcado del cartucho (o arrástralo a esta ventana). Se comprueba su SHA-1 "
                               "y se copia a la carpeta roms. No se incluye ninguna ROM.",
                               "Choose your cartridge dump (or drop it on this window). Its SHA-1 is checked and "
                               "it is copied into the roms folder. No ROM is included."));
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::EndChild();

    ImGui::Spacing();
    bool ready = sState[sSelected].installed && fs::exists(sState[sSelected].exe);
    if (PlayButton(T("JUGAR", "PLAY"), ImVec2(ImGui::GetContentRegionAvail().x, Px(72)), ready))
        sLaunch = true;
    if (!ready) {
        ImGui::PushFont(sFontSmall);
        ImGui::TextColored(kMuted, "%s",
                           sState[sSelected].installed
                               ? T("No se encontró el ejecutable de esta versión.", "This version's executable is missing.")
                               : T("Instala la ROM de esta versión para jugar.", "Install this version's ROM to play."));
        ImGui::PopFont();
    }
    ImGui::Spacing();
    BeginCard("save");
    Eyebrow(T("PARTIDA", "SAVE"));
    if (sSelected == 0) {
        const char* langs[] = { T("Elegir al empezar", "Choose at start"), "English", "Fran\xC3\xA7" "ais", "Deutsch",
                                "Italiano", "Espa\xC3\xB1ol" };
        if (CycleRow(T("Idioma del juego", "Game language"), nullptr, sGameLanguage, 6, langs))
            SaveLauncherState();
    }
    {
        static const char* const banks[] = { "1", "2", "3", "4", "5" };
        int bank = std::clamp(Ini("save_bank"), 1, 5) - 1;
        if (CycleRow(T("Banco de partidas", "Save bank"),
                     T("Cinco bancos de las dos ranuras del juego; el 1 es la partida de siempre.",
                       "Five banks of the game's two slots; bank 1 is the usual save."),
                     bank, 5, banks)) {
            Ini("save_bank") = bank + 1;
            SaveIni();
        }
    }
    {
        std::error_code ec;
        fs::path save = SavePath(sSelected);
        bool has = fs::exists(save, ec);
        char line[160];
        if (has) {
            FILETIME ft, local;
            SYSTEMTIME st;
            WIN32_FILE_ATTRIBUTE_DATA attr;
            if (GetFileAttributesExW(save.c_str(), GetFileExInfoStandard, &attr)) {
                ft = attr.ftLastWriteTime;
                FileTimeToLocalFileTime(&ft, &local);
                FileTimeToSystemTime(&local, &st);
                std::snprintf(line, sizeof(line), "%s %s  -  %02d/%02d/%04d %02d:%02d", T("Partida:", "Save:"),
                              save.filename().string().c_str(), st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
            } else {
                std::snprintf(line, sizeof(line), "%s %s", T("Partida:", "Save:"), save.filename().string().c_str());
            }
        } else {
            std::snprintf(line, sizeof(line), "%s",
                          T("Sin partida guardada para esta versi\xC3\xB3n.", "No save for this version yet."));
        }
        ImGui::PushFont(sFontSmall);
        ImGui::TextColored(has ? kText : kMuted, "%s", line);
        ImGui::PopFont();
        float half = (ImGui::GetContentRegionAvail().x - Px(8)) * 0.5f;
        if (ImGui::Button(T("Abrir carpeta", "Open folder"), ImVec2(half, 0)))
            ShellExecuteW(nullptr, L"explore", sRoot.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        ImGui::SameLine(0, Px(8));
        ImGui::BeginDisabled(!has);
        if (ImGui::Button(T("Copia de seguridad", "Back up save"), ImVec2(half, 0)))
            BackupSave(sSelected);
        ImGui::EndDisabled();
        if (ImGui::Button(T("Importar partida de GBA... (experimental)", "Import GBA save... (experimental)"),
                          ImVec2(-FLT_MIN, 0)))
            ImportSave(window, sSelected);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", T("Una partida de emulador (mGBA, VBA, NO$GBA...) o de GameShark / Action Replay. "
                                      "Se convierte a esta versión si viene de otra región; la actual "
                                      "se guarda antes en backups.",
                                      "A save from an emulator (mGBA, VBA, NO$GBA...) or GameShark / Action Replay. "
                                      "It is converted to this version if it comes from another region; the current "
                                      "one is backed up first."));
    }
    ImGui::EndChild();

    ImGui::EndGroup();
}

static void PageGraphics() {
    const char* views[] = { T("Original 240x160", "Original 240x160"), T("Panorámica 284x160", "Wide 284x160"),
                            T("Ampliada 320x180", "Extended 320x180"), T("Máxima 384x216", "Maximum 384x216"),
                            T("Auto (llenar ventana)", "Auto (fill window)") };
    const char* zooms[] = { T("Cerca", "Near"), T("Normal", "Normal"), T("Lejos", "Far"), T("Muy lejos", "Very far"),
                            T("Lejísimos", "Farthest") };
    const char* scales[] = { T("Entero", "Integer"), T("Ajustar", "Fit"), T("Estirar", "Stretch") };
    const char* filters[] = { T("Nítido", "Sharp"), "Pixel art", T("Suave", "Smooth") };
    const char* huds[] = { T("Original", "Original"), T("Grande", "Large"), T("Más grande", "Larger") };
    const char* narrow[] = { T("Píxel exacto", "Exact pixels"), T("Llenar altura", "Fill height") };
    const char* onoff[] = { T("No", "Off"), T("Sí", "On") };
    const char* scan[] = { T("No", "Off"), "25%", "50%", "75%", "100%" };
    const char* windows[] = { "1x", "2x", "3x", "4x", "5x", "6x", "7x", "8x" };
    bool changed = false;

    BeginCard("display");
    Eyebrow(T("PANTALLA", "DISPLAY"));
    changed |= CycleRow(T("Vista", "View"), T("Auto llena la ventana a escala de píxel entera.",
                                              "Auto fills the window at a whole-number pixel scale."),
                        Ini("view_mode"), 5, views);
    {
        const char* aspects[] = { T("Llenar ventana", "Fill window"), "16:9", "21:9" };
        changed |= CycleRow(T("Relación de aspecto", "Aspect ratio"),
                            T("En monitores más anchos, 16:9 o 21:9 dejan bandas negras a los lados.",
                              "On wider monitors, 16:9 or 21:9 leave black bars at the sides."),
                            Ini("aspect"), 3, aspects);
    }
    changed |= CycleRow(T("Zoom (vista Auto)", "Zoom (Auto view)"), nullptr, Ini("zoom"), 5, zooms);
    changed |= CycleRow(T("Escalado", "Scaling"), nullptr, Ini("scale_mode"), 3, scales);
    changed |= CycleRow(T("Filtro", "Filter"), nullptr, Ini("filter"), 3, filters);
    changed |= CycleRow(T("Tamaño del HUD", "HUD size"), nullptr, Ini("hud_size"), 3, huds);
    changed |= CycleRow(T("Escenas tamaño GBA", "GBA-size scenes"), nullptr, Ini("narrow_fit"), 2, narrow);
    ImGui::EndChild();
    ImGui::Spacing();
    BeginCard("window");
    Eyebrow(T("VENTANA", "WINDOW"));
    changed |= CycleRow(T("Pantalla completa", "Fullscreen"), nullptr, Ini("fullscreen"), 2, onoff);
    int ws = std::clamp(Ini("window_scale"), 1, 8) - 1;
    if (CycleRow(T("Tamaño de ventana", "Window size"), nullptr, ws, 8, windows)) {
        Ini("window_scale") = ws + 1;
        changed = true;
    }
    changed |= CycleRow("VSync", nullptr, Ini("vsync"), 2, onoff);
    int sc = std::clamp(Ini("scanlines") / 25, 0, 4);
    if (CycleRow("Scanlines", nullptr, sc, 5, scan)) {
        Ini("scanlines") = sc * 25;
        changed = true;
    }
    changed |= CycleRow(T("Colores LCD GBA", "GBA LCD colours"), nullptr, Ini("color_correction"), 2, onoff);
    changed |= CycleRow(T("Mostrar FPS", "Show FPS"), nullptr, Ini("show_fps"), 2, onoff);
    ImGui::EndChild();
    if (changed)
        SaveIni();
}

static void PageSound() {
    bool changed = false;
    BeginCard("sound");
    Eyebrow(T("SONIDO", "SOUND"));
    changed |= SliderRow(T("Volumen general", "Master volume"), Ini("master_volume"), 0, 100, "%d%%");
    changed |= SliderRow(T("Volumen música", "Music volume"), Ini("music_volume"), 0, 100, "%d%%");
    changed |= SliderRow(T("Volumen efectos", "Effects volume"), Ini("sfx_volume"), 0, 100, "%d%%");
    ImGui::EndChild();
    if (changed)
        SaveIni();
}

static void PageExtras() {
    bool changed = false;
    BeginCard("extras");
    Eyebrow(T("MEJORAS", "ENHANCEMENTS"));
    {
        static const char* raise[] = { "Off", "4 px", "8 px", "12 px", "16 px" };
        int r = std::clamp(Ini("card_hud_raise") / 4, 0, 4);
        if (CycleRow(T("Subir cartas en combate", "Battle cards raised"),
                     T("La mano de cartas queda en parte bajo el borde de la pantalla; subirla muestra más de cada "
                       "carta.",
                       "The card hand sits partly below the screen edge; raising it shows more of each card."),
                     r, 5, raise)) {
            Ini("card_hud_raise") = r * 4;
            changed = true;
        }
    }
    ImGui::Separator();
    changed |= ToggleRow(T("Voltereta con un botón", "One-button dodge roll"),
                         T("X en el mando, C en el teclado: rueda hacia la dirección que mantienes o hacia donde "
                           "mira el personaje.",
                           "Gamepad X, keyboard C: rolls toward the held direction or the way the character faces."),
                         Ini("dodge_button"));
    ImGui::Separator();
    changed |= ToggleRow(T("Acumular con un botón (L+R)", "One-button stock (L+R)"),
                         T("Y en el mando, D en el teclado: acumula cartas o trucos sin pulsar L y R a la vez.",
                           "Gamepad Y, keyboard D: stock cards or sleights without pressing L and R together."),
                         Ini("stock_button"));
    ImGui::Separator();
    changed |= ToggleRow(T("Devolver cartas acumuladas", "Return stocked cards"),
                         T("Mantén V en el teclado, el stick derecho pulsado o hacia abajo medio segundo: las cartas "
                           "acumuladas de Sora vuelven a la mano, en su sitio de la baraja.",
                           "Hold V on the keyboard, or click or push down the right stick, for half a second: "
                           "Sora's stocked cards go back to the hand, in their places in the deck."),
                         Ini("unstock_button"));
    ImGui::EndChild();
    ImGui::Spacing();
    BeginCard("experimental");
    Eyebrow(T("EXPERIMENTALES", "EXPERIMENTAL"));
    ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
    ImGui::TextWrapped("%s", T("En pruebas: pueden tener fallos visuales.", "Being tested: expect visual glitches."));
    ImGui::PopStyleColor();
    changed |= ToggleRow(T("Texto HD", "HD text"),
                         T("Los diálogos se dibujan con una fuente nítida a la resolución de la ventana, en el "
                           "mismo lugar y colores del juego.",
                           "Dialogue is drawn with a crisp font at the window's resolution, in the game's own "
                           "places and colours."),
                         Ini("hd_text"));
    ImGui::Separator();
#ifdef REKHCOM_SHOW_HD_HUD /* HD HUD pending: hidden, and the game forces it off (KHCOM_HD_HUD_DEV=1 for development) */
    changed |= ToggleRow(T("HUD HD (piloto)", "HD HUD (pilot)"),
                         T("Marco de vida del enemigo redibujado en HD. Piloto para vista panoramica; los demas elementos conservan sus graficos originales.",
                           "Redrawn HD enemy HP frame. Pilot for wide views; other elements keep their original graphics."),
                         Ini("hd_hud"));
    ImGui::Separator();
#endif
    changed |= ToggleRow(T("Estilo HD-2D", "HD-2D look"),
                         T("Campo y combates con profundidad de campo, resplandor, gradación de color y viñeta, al "
                           "estilo de Octopath Traveler. Los sprites quedan nítidos.",
                           "Field and battles with depth of field, bloom, colour grading and a vignette, in the "
                           "manner of Octopath Traveler. Sprites stay sharp."),
                         Ini("hd2d"));
    ImGui::Separator();
    changed |= ToggleRow(T("Audio suave", "Smooth audio"),
                         T("Interpolación cúbica de las muestras y un filtro paso bajo suave: menos aspereza metálica "
                           "en los agudos que dejan las bajas frecuencias de muestreo de la GBA.",
                           "Cubic sample interpolation and a gentle low-pass: less of the metallic edge the GBA's "
                           "low sample rates leave in the treble."),
                         Ini("smooth_audio"));
    ImGui::Separator();
    changed |= ToggleRow(T("Modo Re:CoM", "Re:CoM mode"),
                         T("Al empezar una partida nueva puedes elegir la versión: la original de GBA o la Re:CoM, "
                           "con mejoras de Kingdom Hearts Re:Chain of Memories (PS2) como niveles de dificultad y "
                           "diálogos rápidos. Cada partida recuerda la suya. Apagado, todo queda como el "
                           "cartucho original.",
                           "When starting a new game you can choose the version: the original GBA one or Re:CoM, with "
                           "improvements from Kingdom Hearts Re:Chain of Memories (PS2) such as difficulty levels and "
                           "faster dialogue. Each game remembers its own. Off, everything stays as the original "
                           "cartridge."),
                         Ini("ps2_mode"));
    ImGui::EndChild();
    if (changed)
        SaveIni();
}

/* A binding cell waiting for a key or button (PageControls, MainLoop). */
static std::string sCapture;
static bool sCapturePad;

static const char* PadName(int b) {
    static const char* const names[] = { "A", "B", "X", "Y", "Back", "Guide", "Start", "L3", "R3", "LB", "RB",
                                         "\xE2\x86\x91", "\xE2\x86\x93", "\xE2\x86\x90", "\xE2\x86\x92" };
    if (b < 0)
        return "-";
    if (b < (int)(sizeof(names) / sizeof(names[0])))
        return names[b];
    return SDL_GetGamepadStringForButton((SDL_GamepadButton)b);
}

/* Event for the cell being captured: true when it was used. */
static bool CaptureEvent(const SDL_Event& ev) {
    if (sCapture.empty())
        return false;
    if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
        if (ev.key.key == SDLK_ESCAPE) {
            sCapture.clear();
        } else if (!sCapturePad) {
            Ini(sCapture.c_str()) = ev.key.key == SDLK_DELETE ? 0 : (int)ev.key.key;
            sCapture.clear();
            SaveIni();
        } else if (ev.key.key == SDLK_DELETE) {
            Ini(sCapture.c_str()) = -1;
            sCapture.clear();
            SaveIni();
        }
        return true;
    }
    if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && sCapturePad) {
        Ini(sCapture.c_str()) = ev.gbutton.button;
        sCapture.clear();
        SaveIni();
        return true;
    }
    if (ev.type == SDL_EVENT_GAMEPAD_ADDED)
        SDL_OpenGamepad(ev.gdevice.which);
    return false;
}

static void BindCell(const char* key, bool pad) {
    bool waiting = sCapture == key;
    std::string label;
    if (waiting) {
        label = T("Pulsa...", "Press...");
    } else if (pad) {
        label = PadName(Ini(key));
    } else {
        int k = Ini(key);
        label = k == 0 ? "-" : SDL_GetKeyName((SDL_Keycode)k);
        if (label.empty())
            label = "?";
    }
    label += "##";
    label += key;
    if (waiting)
        ImGui::PushStyleColor(ImGuiCol_Button, kAccent);
    if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0))) {
        sCapture = key;
        sCapturePad = pad;
    }
    if (waiting)
        ImGui::PopStyleColor();
}

static void PageControls() {
    struct Row {
        const char* es;
        const char* en;
        const char* name;
    };
    static const Row rows[] = {
        { "A", "A", "a" },
        { "B", "B", "b" },
        { "L", "L", "l" },
        { "R", "R", "r" },
        { "Start", "Start", "start" },
        { "Select", "Select", "select" },
        { "Arriba", "Up", "up" },
        { "Abajo", "Down", "down" },
        { "Izquierda", "Left", "left" },
        { "Derecha", "Right", "right" },
        { "L + R (acumular)", "L + R (stock)", "stock" },
        { "Voltereta", "Dodge roll", "dodge" },
        { "Devolver acumuladas", "Return stocked", "unstock" },
    };
    BeginCard("controls");
    Eyebrow(T("CONTROLES", "CONTROLS"));
    ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
    ImGui::TextWrapped("%s", T("Haz clic en una casilla y pulsa la tecla o el botón. Supr la deja vacía, Esc cancela. "
                               "Los gatillos y el stick izquierdo funcionan siempre como L/R y cruceta; el stick derecho "
                               "cambia de carta (izquierda/derecha = L/R), arriba acumula (L+R) y abajo devuelve las acumuladas.",
                               "Click a cell and press the key or button. Delete clears it, Esc cancels. The "
                               "triggers and the left stick always work as L/R and the d-pad; the right stick "
                               "changes cards (left/right = L/R), up stocks (L+R) and down returns the stocked cards."));
    ImGui::PopStyleColor();
    if (ImGui::BeginTable("binds", 4, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_PadOuterX)) {
        ImGui::TableSetupColumn(T("Acción", "Action"), ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn(T("Teclado", "Keyboard"), ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn(T("Teclado 2", "Keyboard 2"), ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn(T("Mando", "Gamepad"), ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();
        for (const Row& r : rows) {
            std::string kb = std::string("kb_") + r.name, kb2 = kb + "_2", pad = std::string("pad_") + r.name;
            ImGui::TableNextRow(0, Px(36));
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(T(r.es, r.en));
            ImGui::TableNextColumn();
            BindCell(kb.c_str(), false);
            ImGui::TableNextColumn();
            BindCell(kb2.c_str(), false);
            ImGui::TableNextColumn();
            BindCell(pad.c_str(), true);
        }
        ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button(T("Restablecer controles", "Reset controls"))) {
        for (const Setting& st : kSettings) {
            if (std::strncmp(st.key, "kb_", 3) == 0 || std::strncmp(st.key, "pad_", 4) == 0)
                Ini(st.key) = st.def;
        }
        SaveIni();
    }
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, kMuted);
    ImGui::TextWrapped("%s", T("Fijos: Opciones F1 / Esc o Start + Select (en el inicio, solo Select), pantalla completa F11 o Alt+Enter, "
                               "estados F5 / F9.",
                               "Fixed: options F1 / Esc or Start + Select (on the title screen, Select alone), fullscreen F11 or Alt+Enter, states "
                               "F5 / F9."));
    ImGui::PopStyleColor();
    ImGui::EndChild();
}

/* ---- header and frame ---------------------------------------------------------- */

static void Header() {
    ImGui::PushFont(sFontSmall);
    ImGui::TextColored(kAccent, "REKHCOM  \xC2\xB7  DECOMP PORT  \xC2\xB7  " REKHCOM_VERSION);
    ImGui::SameLine(0, Px(14));
    ImGui::TextColored(kText, "%s MatyMeatBoy", T("por", "by"));
    {
        const char* uis[] = { "", "Idioma: Espa\xC3\xB1ol", "Language: English" };
        float w = Px(190);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - w);
        ImGui::PushStyleColor(ImGuiCol_Button, kControl);
        if (ImGui::Button(uis[sUiLanguage], ImVec2(w, 0))) {
            sUiLanguage = sUiLanguage == 1 ? 2 : 1;
            ApplyUiLanguage();
            SaveLauncherState();
        }
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    if (sLogo != 0) {
        /* the game's logo in place of the title */
        float lh = Px(124), lw = lh * sLogoW / sLogoH;
        ImGui::Image((ImTextureID)(intptr_t)sLogo, ImVec2(lw, lh));
    } else {
        ImGui::PushFont(sFontTitle);
        ImGui::TextUnformatted("Kingdom Hearts: Chain of Memories");
        ImGui::PopFont();
    }

    const char* tabs[PAGE_COUNT] = { T("Inicio", "Home"), T("Gráficos", "Graphics"), T("Sonido", "Sound"),
                                     T("Mejoras", "Enhancements"), T("Controles", "Controls") };
    ImGui::Spacing();
    for (int i = 0; i < PAGE_COUNT; i++) {
        bool active = sPage == i;
        /* the current page: black like the debug menu's selected row */
        ImGui::PushStyleColor(ImGuiCol_Button, active ? kText : kControl);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? kText : kControlHover);
        ImGui::PushStyleColor(ImGuiCol_Text, active ? kInk : kText);
        if (ImGui::Button(tabs[i]))
            sPage = i;
        ImGui::PopStyleColor(3);
        if (i + 1 < PAGE_COUNT)
            ImGui::SameLine(0, Px(6));
    }
    ImGui::Spacing();
    ImGui::Spacing();
}

static void Footer() {
    if (!sNotice.empty() && ImGui::GetTime() - sNoticeTime < 8.0) {
        ImGui::PushFont(sFontSmall);
        ImGui::TextColored(sNoticeError ? kBad : kGood, "%s", sNotice.c_str());
        ImGui::PopFont();
    }
}

static void Frame(SDL_Window* window) {
    ImGuiIO& io = ImGui::GetIO();
    Background(io.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("decompui", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);
    Header();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("page", ImVec2(0, -Px(28)), ImGuiChildFlags_None, ImGuiWindowFlags_None);
    ImGui::PopStyleColor();
    switch (sPage) {
        case PAGE_HOME:
            PageHome(window);
            break;
        case PAGE_GRAPHICS:
            PageGraphics();
            break;
        case PAGE_SOUND:
            PageSound();
            break;
        case PAGE_EXTRAS:
            PageExtras();
            break;
        case PAGE_CONTROLS:
            PageControls();
            break;
    }
    ImGui::EndChild();
    Footer();
    ImGui::End();
}

/* ---- launching --------------------------------------------------------------- */

static std::wstring Quote(const std::wstring& s) {
    std::wstring out = L"\"";
    unsigned slashes = 0;
    for (wchar_t c : s) {
        if (c == L'\\') {
            slashes++;
            continue;
        }
        out.append(c == L'"' ? slashes * 2 + 1 : slashes, L'\\');
        out += c;
        slashes = 0;
    }
    out.append(slashes * 2, L'\\');
    return out + L"\"";
}

/* Runs the game with the shared data folder; the launcher waits hidden. */
static void Launch(SDL_Window* window) {
    const VersionState& st = sState[sSelected];
    std::wstring cmd = Quote(st.exe.wstring()) + L" --data-dir " + Quote(sRoot.wstring()) + L" --rom " +
                       Quote(RomPath(sSelected).wstring());
    if (sSelected == 0 && sGameLanguage > 0) {
        std::string id = kGameLanguageIds[sGameLanguage];
        cmd += L" --language " + std::wstring(id.begin(), id.end());
    }
    STARTUPINFOW si = {};
    PROCESS_INFORMATION pi = {};

    si.cb = sizeof(si);
    SaveIni();
    SaveLauncherState();
    if (!CreateProcessW(st.exe.c_str(), cmd.data(), nullptr, nullptr, FALSE, 0, nullptr,
                        st.exe.parent_path().c_str(), &si, &pi)) {
        Notice(T("No se pudo iniciar el juego.", "Could not start the game."), true);
        return;
    }
    CloseHandle(pi.hThread);
    SDL_HideWindow(window);
    while (WaitForSingleObject(pi.hProcess, 100) == WAIT_TIMEOUT) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
        }
    }
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);
    LoadIni();
    if (code != 0)
        Notice(T("El juego se cerró con un error; revisa kh_com.log.", "The game closed with an error; see kh_com.log."),
               true);
}

/* ---- test harness ------------------------------------------------------------
 * DECOMPUI_SHOT=dir  render every page off screen (hidden window) and save
 *                    dir/decompui_<page>.bmp, then exit.
 * DECOMPUI_INSTALL=path  install that ROM before the first frame.
 */
typedef void(APIENTRY* GenFn)(GLsizei, GLuint*);
typedef void(APIENTRY* BindFn)(GLenum, GLuint);
typedef void(APIENTRY* StorageFn)(GLenum, GLenum, GLsizei, GLsizei);
typedef void(APIENTRY* AttachRbFn)(GLenum, GLenum, GLenum, GLuint);

static bool ShotFrames(SDL_Window* window, const char* dir) {
    GenFn genFb = (GenFn)SDL_GL_GetProcAddress("glGenFramebuffers");
    BindFn bindFb = (BindFn)SDL_GL_GetProcAddress("glBindFramebuffer");
    GenFn genRb = (GenFn)SDL_GL_GetProcAddress("glGenRenderbuffers");
    BindFn bindRb = (BindFn)SDL_GL_GetProcAddress("glBindRenderbuffer");
    StorageFn storage = (StorageFn)SDL_GL_GetProcAddress("glRenderbufferStorage");
    AttachRbFn attach = (AttachRbFn)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
    int w, h;
    GLuint fb, rb;

    if (!genFb || !bindFb || !genRb || !bindRb || !storage || !attach)
        return false;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    genFb(1, &fb);
    bindFb(0x8D40 /* GL_FRAMEBUFFER */, fb);
    genRb(1, &rb);
    bindRb(0x8D41 /* GL_RENDERBUFFER */, rb);
    storage(0x8D41, GL_RGBA8, w, h);
    attach(0x8D40, 0x8CE0 /* GL_COLOR_ATTACHMENT0 */, 0x8D41, rb);
    std::vector<unsigned char> px((size_t)w * h * 4);
    for (int page = 0; page < PAGE_COUNT; page++) {
        sPage = page;
        for (int i = 0; i < 3; i++) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            ImGui::PushFont(sFontBody);
            Frame(window);
            ImGui::PopFont();
            ImGui::Render();
            glViewport(0, 0, w, h);
            glClearColor(kInk.x, kInk.y, kInk.z, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        SDL_Surface* s = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ABGR8888);
        for (int y = 0; y < h; y++)
            std::memcpy((unsigned char*)s->pixels + (size_t)y * s->pitch, px.data() + (size_t)(h - 1 - y) * w * 4,
                        (size_t)w * 4);
        char name[64];
        std::snprintf(name, sizeof(name), "decompui_%d.bmp", page);
        SDL_SaveBMP(s, (fs::u8path(dir) / name).string().c_str());
        SDL_DestroySurface(s);
    }
    return true;
}

static fs::path ExeDir() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return fs::path(path).parent_path();
}

int main(int, char**) {
    sRoot = ExeDir();
    ApplyUiLanguage();
    ScanInstall();
    LoadIni();
    LoadLauncherState();
    SaveLauncherState(); /* the first time, keeps the language chosen from the system */
    if (!sState[sSelected].installed) {
        for (int v = 0; v < kVersionCount; v++)
            if (sState[v].installed) {
                sSelected = v;
                break;
            }
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        return 1;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    sScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (sScale < 1.0f)
        sScale = 1.0f;
    const char* shotDir = SDL_getenv("DECOMPUI_SHOT");
    SDL_Window* window = SDL_CreateWindow("ReKHCoM", (int)Px(1180), (int)Px(760),
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                              (shotDir != nullptr ? SDL_WINDOW_HIDDEN : 0));
    if (window == nullptr)
        return 1;
    SDL_SetWindowMinimumSize(window, (int)Px(960), (int)Px(640));
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    ApplyStyle();
    LoadFonts();
    ImGui_ImplSDL3_InitForOpenGL(window, gl);
    ImGui_ImplOpenGL3_Init("#version 130");
    LoadBoxArt();
    if (const char* rom = SDL_getenv("DECOMPUI_INSTALL")) {
        InstallRom(fs::u8path(rom));
    }

    bool running = shotDir == nullptr || !ShotFrames(window, shotDir);
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (CaptureEvent(ev))
                continue;
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT || ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                running = false;
            if (ev.type == SDL_EVENT_DROP_FILE && ev.drop.data != nullptr)
                InstallRom(fs::u8path(ev.drop.data));
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(50);
            continue;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::PushFont(sFontBody);
        Frame(window);
        ImGui::PopFont();
        ImGui::Render();
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(kInk.x, kInk.y, kInk.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        if (sLaunch) {
            sLaunch = false;
            Launch(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
