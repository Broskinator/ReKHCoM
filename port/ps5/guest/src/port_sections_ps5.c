/*
 * Locates the game's globals (sections khdata / khbss, see port/game_prefix.h)
 * from the linker script of the game image (guest.ld).
 */
#include <string.h>

#include "port/port.h"

extern char __start_khdata[], __stop_khdata[], __start_khbss[], __stop_khbss[];

void PortGameSections(char** data, size_t* dataSize, char** bss, size_t* bssSize) {
    *data = __start_khdata;
    *dataSize = (size_t)(__stop_khdata - __start_khdata);
    *bss = __start_khbss;
    *bssSize = (size_t)(__stop_khbss - __start_khbss);
}
