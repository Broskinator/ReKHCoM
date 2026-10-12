/*
 * Development aid: when KHCOM_WATCHDOG is set, a background thread samples
 * the game thread every few seconds and logs its call stack (frame-pointer
 * walk) so hangs can be located with llvm-addr2line.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "port/port.h"

static HANDLE sGameThread;

static DWORD WINAPI Watch(LPVOID arg) {
    int period = atoi((const char*)arg);

    if (period <= 0)
        period = 3;
    for (;;) {
        CONTEXT ctx;
        int i;

        Sleep(period * 1000);
        SuspendThread(sGameThread);
        ctx.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
        if (GetThreadContext(sGameThread, &ctx)) {
            uint32_t ebp = ctx.Ebp;
            PortLog("watchdog: eip=%08lx", (unsigned long)ctx.Eip);
            for (i = 0; i < 24 && ebp >= 0x10000; i++) {
                uint32_t ret = 0, next = 0;
                if (IsBadReadPtr((void*)(uintptr_t)ebp, 8))
                    break;
                next = ((uint32_t*)(uintptr_t)ebp)[0];
                ret = ((uint32_t*)(uintptr_t)ebp)[1];
                PortLog("watchdog:   %08lx", (unsigned long)ret);
                if (next <= ebp)
                    break;
                ebp = next;
            }
        }
        ResumeThread(sGameThread);
    }
    return 0;
}

static void LogStack(uint32_t eip, uint32_t ebp) {
    int i;

    PortLog("  at %08lx", (unsigned long)eip);
    for (i = 0; i < 32 && ebp >= 0x10000; i++) {
        uint32_t next, ret;
        if (IsBadReadPtr((void*)(uintptr_t)ebp, 8))
            break;
        next = ((uint32_t*)(uintptr_t)ebp)[0];
        ret = ((uint32_t*)(uintptr_t)ebp)[1];
        PortLog("  at %08lx", (unsigned long)ret);
        if (next <= ebp)
            break;
        ebp = next;
    }
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* info) {
    EXCEPTION_RECORD* r = info->ExceptionRecord;
    CONTEXT* c = info->ContextRecord;

    PortLog("CRASH: exception %08lx at %08lx (access %s %08lx)", (unsigned long)r->ExceptionCode,
            (unsigned long)c->Eip, r->NumberParameters > 0 && r->ExceptionInformation[0] ? "write" : "read",
            r->NumberParameters > 1 ? (unsigned long)r->ExceptionInformation[1] : 0ul);
    PortLog("  eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx ebp=%08lx esp=%08lx",
            c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp);
    LogStack(c->Eip, c->Ebp);
    return EXCEPTION_CONTINUE_SEARCH;
}

void PortCrashHandlerInstall(void) {
    SetUnhandledExceptionFilter(CrashHandler);
}

void PortWatchdogStart(void) {
    const char* env = getenv("KHCOM_WATCHDOG");

    if (env == NULL)
        return;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &sGameThread, 0, FALSE,
                    DUPLICATE_SAME_ACCESS);
    CreateThread(NULL, 0, Watch, (LPVOID)env, 0, NULL);
}
