/*
 * Locates the game's globals (sections khdata / khbss, see
 * port/game_prefix.h) from the executable's own PE section table.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

#include "port/port.h"

static int FindSection(const char* name, char** start, size_t* size) {
    const uint8_t* base = (const uint8_t*)GetModuleHandleA(NULL);
    const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)base;
    const IMAGE_NT_HEADERS* nt = (const IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    WORD i;

    for (i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
        if (strncmp((const char*)sec->Name, name, IMAGE_SIZEOF_SHORT_NAME) == 0) {
            *start = (char*)(base + sec->VirtualAddress);
            *size = sec->Misc.VirtualSize;
            return 1;
        }
    }
    *start = NULL;
    *size = 0;
    return 0;
}

void PortGameSections(char** data, size_t* dataSize, char** bss, size_t* bssSize) {
    if (!FindSection("khdata", data, dataSize) || !FindSection("khbss", bss, bssSize))
        PortFatal("The executable is missing its game data sections (khdata/khbss).");
}
