/*
 * Force-included into every game translation unit of the PC build.
 *
 * Puts the game's mutable globals in their own PE sections (khdata/khbss) so
 * soft reset and save states can find exactly them (see port_sections.c).
 * Mach-O objects (the Switch build's 32-bit-pointer code) name a segment too.
 */
#ifdef __MACH__
#pragma clang section bss = "__DATA,khbss" data = "__DATA,khdata"
#else
#pragma clang section bss = "khbss" data = "khdata"
#endif
