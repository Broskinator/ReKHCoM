#ifndef G_SETJMP_H
#define G_SETJMP_H
#ifdef __aarch64__
typedef unsigned long long jmp_buf[22]; /* x19-x30, sp, d8-d15 */
#else
typedef unsigned long long jmp_buf[8];
#endif
int setjmp(jmp_buf);
void longjmp(jmp_buf, int) __attribute__((noreturn));
#endif
