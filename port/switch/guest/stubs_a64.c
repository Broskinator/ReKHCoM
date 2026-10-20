/* One stub per host function: loads the table index and jumps to the trampoline (crt0_a64.S). The index is the
 * entry's position in bridge_list.h, the same order as the host's table (HF_*). */
#include "bridge.h"

#define STR2(x) #x
#define STR(x) STR2(x)
#define X(name) \
    __asm__(".text\n.p2align 2\n.globl _h_" #name "\n_h_" #name ":\n\tmov w16, #" STR(__COUNTER__) "\n\tb ___host_call\n");
#include "bridge_list.h"
#undef X

uint64_t gHostTable[HF_COUNT];

/* What the host reads to start the image (port/switch/host): its own segment, so it is found by name. */
extern char rk_heap[], rk_stack[];
extern char gPortBiosArea[];
extern int switch_boot(uint32_t stackTop);

__attribute__((section("__RKHDR,__header"), used)) const uint32_t gSwitchGuestHeader[9] = {
    PS5G_MAGIC,
    0, /* imageEnd: the host knows it from the segments */
    (uint32_t)(uintptr_t)switch_boot,
    (uint32_t)(uintptr_t)gHostTable,
    (uint32_t)(uintptr_t)rk_heap,
    (uint32_t)(uintptr_t)(rk_heap + 0x06000000),
    (uint32_t)(uintptr_t)(rk_stack + 0x00800000),
    0, /* regionEnd: idem */
    (uint32_t)(uintptr_t)gPortBiosArea,
};
