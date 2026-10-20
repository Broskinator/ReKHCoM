#ifndef GUARD_GBA_GBA_SPACE_H
#define GUARD_GBA_GBA_SPACE_H

/*
 * Where each region of the GBA address space is. On the GBA, and in the ports that map the regions at their
 * hardware addresses (Windows, Linux, Android, web, PS5, PS4), these are the bus addresses. With PORT_GBA_RELOC
 * (consoles where those addresses cannot be mapped: Vita, Switch) every region is an array of the program and the
 * bases below are their link-time addresses, so the game's register macros and static tables keep compiling.
 * Pointers stored inside ROM data are moved when the ROM is loaded (port_memory.c, the list of the ROM's pointer
 * words), and GbaSpacePtr() translates a raw bus address met at run time.
 */
#if defined(PLATFORM_PC) && defined(PORT_GBA_RELOC)
extern unsigned char gGbaEwram[], gGbaIwram[], gGbaIo[], gGbaPltt[], gGbaVram[], gGbaOam[], gGbaRom[], gGbaSram[];
#define GBA_EWRAM_BASE ((__UINTPTR_TYPE__)gGbaEwram)
#define GBA_IWRAM_BASE ((__UINTPTR_TYPE__)gGbaIwram)
#define GBA_IO_BASE ((__UINTPTR_TYPE__)gGbaIo)
#define GBA_PLTT_BASE ((__UINTPTR_TYPE__)gGbaPltt)
#define GBA_VRAM_BASE ((__UINTPTR_TYPE__)gGbaVram)
#define GBA_OAM_BASE ((__UINTPTR_TYPE__)gGbaOam)
#define GBA_ROM_BASE ((__UINTPTR_TYPE__)gGbaRom)
#define GBA_SRAM_BASE ((__UINTPTR_TYPE__)gGbaSram)
/* a bus address (0x02000000-0x0EFFFFFF, mirrors included) to where it is; 0 when no region decodes it */
void* GbaSpacePtr(unsigned int address);
#define GBA_BUS_PTR(type, address) ((type)GbaSpacePtr((unsigned int)(address)))
#else
#define GBA_EWRAM_BASE 0x02000000u
#define GBA_IWRAM_BASE 0x03000000u
#define GBA_IO_BASE 0x04000000u
#define GBA_PLTT_BASE 0x05000000u
#define GBA_VRAM_BASE 0x06000000u
#define GBA_OAM_BASE 0x07000000u
#define GBA_ROM_BASE 0x08000000u
#define GBA_SRAM_BASE 0x0E000000u
#define GBA_BUS_PTR(type, address) ((type)(address))
#endif

#endif /* GUARD_GBA_GBA_SPACE_H */
