/*
 * Force-included into every game translation unit of the PC build.
 *
 * Puts the game's mutable globals in their own PE sections (khdata/khbss) so
 * soft reset and save states can find exactly them (see port_sections.c).
 */
#pragma clang section bss = "khbss" data = "khdata"
