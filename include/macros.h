#ifndef GUARD_MACROS_H
#define GUARD_MACROS_H

#define INCLUDE_ASM(path) asm(".include \"" path "\"")

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define STATIC_ASSERT(expr, id) typedef char id[(expr) ? 1 : -1];

#ifdef PLATFORM_PC
/* The PC build keeps work RAM variables as ordinary native globals. */
#define EWRAM_COMMON(align) __attribute__((aligned(align))) = {0}
#define IWRAM_DATA(align) __attribute__((aligned(align))) = {0}
#define IWRAM_COMMON(align) __attribute__((aligned(align))) = {0}
#else
#define EWRAM_COMMON(align) __attribute__((section(".ewram_common." STRINGIFY(__LINE__)), aligned(align))) = {0}
#define IWRAM_DATA(align) __attribute__((section(".iwram." STRINGIFY(__LINE__)), aligned(align))) = {0}
#define IWRAM_COMMON(align) __attribute__((section(".iwram_common." STRINGIFY(__LINE__)), aligned(align))) = {0}
#endif

#endif /* GUARD_MACROS_H */
