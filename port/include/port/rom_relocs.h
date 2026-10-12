#ifndef GUARD_PORT_ROM_RELOCS_H
#define GUARD_PORT_ROM_RELOCS_H

#include <stdint.h>

/* A ROM word that must hold the address of a natively compiled symbol. */
typedef struct PortRomReloc {
    uint32_t romAddress;
    const void* target;
} PortRomReloc;

extern const PortRomReloc gPortRomRelocs[];

#endif
