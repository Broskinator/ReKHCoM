/* Bounds of the game's .data / .bss (see port/game_prefix.h). The linker
   orders grouped sections by the text after '$'. */
__attribute__((section(".data$kh0"), used)) char __kh_data_start[4] = { 1 };
__attribute__((section(".data$kh2"), used)) char __kh_data_end[4] = { 1 };
__attribute__((section(".bss$kh0"), used)) char __kh_bss_start[4];
__attribute__((section(".bss$kh2"), used)) char __kh_bss_end[4];
