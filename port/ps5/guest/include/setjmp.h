#ifndef G_SETJMP_H
#define G_SETJMP_H
typedef unsigned long long jmp_buf[8];
int setjmp(jmp_buf);
void longjmp(jmp_buf, int) __attribute__((noreturn));
#endif
