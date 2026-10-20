/* Test builds only (build_android.py --ubsan): the sanitizer's minimal-runtime hooks write one logcat line
   per distinct code address, then execution continues. Finds what differs from the GBA/x86 desktop port:
   shifts by 32 or more (ARM gives 0, x86 masks the count), divisions by zero, NULL and misaligned
   accesses. Read with: adb logcat -s ReKHCoM-UBSAN, then llvm-addr2line the addresses. */
#define _GNU_SOURCE
#include <android/log.h>
#include <dlfcn.h>
#include <stdint.h>

static void Report(const char* kind, void* pc) {
    static void* seen[512];
    static int count;
    int i;
    for (i = 0; i < count; ++i)
        if (seen[i] == pc) return;
    if (count < 512) seen[count++] = pc;
    Dl_info info;
    uintptr_t offset = dladdr(pc, &info) ? (uintptr_t)pc - (uintptr_t)info.dli_fbase : (uintptr_t)pc;
    __android_log_print(ANDROID_LOG_WARN, "ReKHCoM-UBSAN", "%s pc=0x%x", kind, (unsigned)offset);
}

void __ubsan_handle_shift_out_of_bounds_minimal(void) { Report("shift", __builtin_return_address(0)); }
void __ubsan_handle_divrem_overflow_minimal(void) { Report("divide", __builtin_return_address(0)); }
void __ubsan_handle_type_mismatch_minimal(void) { Report("null-or-misaligned", __builtin_return_address(0)); }
void __ubsan_handle_shift_out_of_bounds_minimal_abort(void) { Report("shift", __builtin_return_address(0)); }
void __ubsan_handle_divrem_overflow_minimal_abort(void) { Report("divide", __builtin_return_address(0)); }
void __ubsan_handle_type_mismatch_minimal_abort(void) { Report("null-or-misaligned", __builtin_return_address(0)); }
