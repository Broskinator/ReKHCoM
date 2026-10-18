#include "local_achievements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

const KhLocalDefinition gKhLocalDefinitions[KH_LOCAL_COUNT] = {
    {"Primeros recuerdos", "First memories", "Alcanza el nivel 10.", "Reach level 10."},
    {"Corazón en marcha", "Heart in motion", "Alcanza el nivel 25.", "Reach level 25."},
    {"Recuerdos fuertes", "Stronger memories", "Alcanza el nivel 50.", "Reach level 50."},
    {"El límite del corazón", "The heart's limit", "Alcanza el nivel 99.", "Reach level 99."},
    {"Un corazón resistente", "A resilient heart", "Consigue 200 puntos de vida máxima.", "Reach 200 maximum HP."},
    {"Espacio en la baraja", "Room in the deck", "Consigue 500 CP con Sora.", "Reach 500 CP as Sora."},
    {"Ahorros moguri", "Moogle savings", "Reúne 1000 puntos moguri a la vez.", "Hold 1,000 Moogle Points."},
    {"Fortuna moguri", "Moogle fortune", "Reúne 10000 puntos moguri a la vez.", "Hold 10,000 Moogle Points."},
    {"Un amigo de otro mundo", "A friend from another world", "Consigue una carta de amigo de un mundo.", "Obtain a world friend card."},
    {"Una baraja de trucos", "A deck of sleights", "Aprende diez trucos diferentes.", "Learn ten different sleights."},
    {"Coleccionista", "Collector", "Registra veinte clases de cartas diferentes.", "Record twenty different card kinds."},
    {"La promesa de Sora", "Sora's promise", "Completa la historia de Sora.", "Complete Sora's story."},
    {"El camino de Riku", "Riku's path", "Completa la historia de Riku.", "Complete Riku's story."},
    {"Cinco pisos de recuerdos", "Five floors of memories", "Alcanza el piso 5 con Sora.", "Reach floor 5 as Sora."},
    {"Más alto en el castillo", "Higher in the castle", "Alcanza el piso 9 con Sora.", "Reach floor 9 as Sora."},
    {"La última planta", "The final floor", "Alcanza el piso 13 con Sora.", "Reach floor 13 as Sora."}
};

static unsigned PopCount(uint64_t value) {
    unsigned n = 0;
    while (value) { value &= value - 1; ++n; }
    return n;
}

uint32_t KhLocalObserve(KhLocalState* state, const KhLocalSnapshot* s, uint64_t now) {
    uint32_t met = 0, added;
    int i;
    if (!s->playing) return 0;
#define WHEN(id, condition) do { if (condition) met |= 1u << (id); } while (0)
    WHEN(0, s->level >= 10); WHEN(1, s->level >= 25);
    WHEN(2, s->level >= 50); WHEN(3, s->level >= 99);
    WHEN(4, s->max_hp >= 200); WHEN(5, !(s->flags & 8) && s->cp >= 500);
    WHEN(6, s->moogle_points >= 1000); WHEN(7, s->moogle_points >= 10000);
    WHEN(8, !(s->flags & 8) && (s->friends & 0x7C) != 0);
    WHEN(9, !(s->flags & 8) && PopCount(s->learned) + PopCount(s->learned2) >= 10);
    WHEN(10, !(s->flags & 8) && PopCount(s->card_kinds) >= 20);
    WHEN(11, s->flags & 0x20); WHEN(12, s->flags & 0x800);
    WHEN(13, !(s->flags & 8) && s->floor >= 4);
    WHEN(14, !(s->flags & 8) && s->floor >= 8);
    WHEN(15, !(s->flags & 8) && s->floor >= 12);
#undef WHEN
    /* Preserve the highest observed condition across character/save changes. */
    added = state->initialized ? met & ~state->previous & ~state->unlocked : 0;
    state->previous |= met;
    state->initialized = 1;
    state->unlocked |= added;
    for (i = 0; i < KH_LOCAL_COUNT; ++i) if (added & (1u << i)) state->when[i] = now;
    return added;
}

static FILE* Open(const char* path, const char* mode) {
#ifdef _WIN32
    wchar_t wide[1400], wmode[8];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 1400) ||
        !MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 8)) return NULL;
    return _wfopen(wide, wmode);
#else
    return fopen(path, mode);
#endif
}

/* Keep the storage parser available on small console guest C libraries too. */
static int ReadNumber(FILE* file, unsigned base, uint64_t* result) {
    char line[80];
    const char* p;
    uint64_t value = 0;
    int digits = 0;
    if (!fgets(line, sizeof(line), file)) return 0;
    for (p = line; *p && *p != '\r' && *p != '\n'; ++p) {
        unsigned digit;
        if (*p >= '0' && *p <= '9') digit = (unsigned)(*p - '0');
        else if (*p >= 'a' && *p <= 'f') digit = (unsigned)(*p - 'a' + 10);
        else if (*p >= 'A' && *p <= 'F') digit = (unsigned)(*p - 'A' + 10);
        else return 0;
        if (digit >= base || value > (UINT64_MAX - digit) / base) return 0;
        value = value * base + digit; ++digits;
    }
    if (!digits) return 0;
    *result = value; return 1;
}

int KhLocalLoad(KhLocalState* state, const char* path) {
    char header[32];
    uint64_t mask = 0;
    KhLocalState loaded;
    FILE* file = Open(path, "r");
    int i, ok = 1;
    memset(state, 0, sizeof(*state));
    if (!file) return 0;
    memset(&loaded, 0, sizeof(loaded));
    if (!fgets(header, sizeof(header), file) || strcmp(header, "KHLOCAL1\n") != 0 ||
        !ReadNumber(file, 16, &mask) || mask >> KH_LOCAL_COUNT) ok = 0;
    for (i = 0; ok && i < KH_LOCAL_COUNT; ++i) {
        if (!ReadNumber(file, 10, &loaded.when[i])) ok = 0;
    }
    if (ok) { loaded.unlocked = (uint32_t)mask; *state = loaded; }
    fclose(file);
    return ok;
}

int KhLocalSave(const KhLocalState* state, const char* path) {
    char temp[1400];
    FILE* file;
    int i, ok;
    if (snprintf(temp, sizeof(temp), "%s.tmp", path) >= (int)sizeof(temp)) return 0;
    file = Open(temp, "w");
    if (!file) return 0;
    ok = fprintf(file, "KHLOCAL1\n%lx\n", (unsigned long)state->unlocked) > 0;
    for (i = 0; ok && i < KH_LOCAL_COUNT; ++i)
        ok = fprintf(file, "%llu\n", (unsigned long long)state->when[i]) > 0;
    if (fclose(file) != 0) ok = 0;
    if (!ok) return 0;
#ifdef _WIN32
    {
        wchar_t from[1400], to[1400];
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, temp, -1, from, 1400) ||
            !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, to, 1400)) return 0;
        return MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    }
#else
    return rename(temp, path) == 0;
#endif
}
