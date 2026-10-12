/*
 * Force-included into every game translation unit of the PC build.
 *
 * Groups the game's mutable globals between the markers in port_markers.c so
 * a soft reset can restore them, like the BIOS clearing work RAM on hardware.
 */
#pragma clang section bss = ".bss$kh1" data = ".data$kh1"
