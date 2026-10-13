/*
 * Stray pointers, as the GBA bus handles them.
 *
 * On the cartridge, a read through a NULL (or nearly NULL) pointer lands in
 * the BIOS and returns the open-bus value; a write there is ignored. The
 * original game relies on this in places (marked "@bug ... (NULL read)" and
 * wrapped in BIOS_PTR() where they are known), sometimes then using the
 * open-bus value itself as a pointer: the bus ignores the top address bits,
 * so 0xE3A02020 is an SRAM mirror. On Windows those accesses fault: the low
 * 64 KiB are never mapped, and nothing is mapped outside the GBA space.
 *
 * This vectored handler makes them behave like the GBA: the faulting
 * instruction's address register is moved so that it points at the GBA
 * location the bus would decode (gPortBiosArea, the stand-in BIOS area
 * refreshed every frame, for the BIOS and open-bus regions), the CPU
 * re-executes the instruction one step, and the register is moved back.
 * Only the ModRM/SIB bytes are decoded, to find that register; the CPU does
 * the rest, flags included. Each site is logged once ("bios: ...") so it can
 * be marked in the sources.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "port/port.h"

extern uint8_t gPortBiosArea[0x10000];

#define BIOS_AREA_SIZE 0x10000u
#define GBA_SPACE_LOW 0x02000000u
#define GBA_SPACE_HIGH 0x0E010000u
#define LOGGED_MAX 128

typedef struct Pending {
    int reg;          /* CONTEXT register index, -1 when none */
    uint32_t delta;   /* added to it for the step */
    uint32_t target;  /* its value after the move */
    DWORD thread;
} Pending;

static Pending sPending = { -1, 0, 0, 0 };
static uint32_t sLogged[LOGGED_MAX];
static int sLoggedCount;

static DWORD* Reg(CONTEXT* c, int r) {
    switch (r) {
        case 0: return &c->Eax;
        case 1: return &c->Ecx;
        case 2: return &c->Edx;
        case 3: return &c->Ebx;
        case 4: return &c->Esp;
        case 5: return &c->Ebp;
        case 6: return &c->Esi;
        default: return &c->Edi;
    }
}

/* One-byte opcodes followed by a ModRM byte. */
static int HasModRm1(uint8_t op) {
    if (op < 0x40)
        return (op & 7) < 4; /* ALU r/m forms; 0F is handled by the caller */
    switch (op) {
        case 0x62: case 0x63: case 0x69: case 0x6B: case 0xC0: case 0xC1: case 0xC4: case 0xC5:
        case 0xC6: case 0xC7: case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xF6: case 0xF7:
        case 0xFE: case 0xFF:
            return 1;
    }
    return (op >= 0x80 && op <= 0x8F) || (op >= 0xD8 && op <= 0xDF);
}

/* Two-byte (0F xx) opcodes without a ModRM byte. */
static int NoModRm2(uint8_t op) {
    return (op >= 0x05 && op <= 0x0B) || op == 0x0E || (op >= 0x30 && op <= 0x37) || op == 0x77
           || (op >= 0x80 && op <= 0x8F) || (op >= 0xA0 && op <= 0xA2) || (op >= 0xA8 && op <= 0xAA)
           || op >= 0xC8;
}

/*
 * Picks the register to move and by how much so the instruction's effective
 * address shifts by exactly `shift`. Returns 0 when the instruction cannot be
 * handled.
 */
/* Where the GBA bus sends address `a`: its region's mirror, or the BIOS
   stand-in for the BIOS and open-bus regions (and for writes to the ROM,
   which the cartridge ignores). */
static uint32_t BusTarget(uint32_t a, int write) {
    uint32_t bios = (uint32_t)(uintptr_t)gPortBiosArea + ((a & 0xFFFFu) > 0xFFE0u ? (a & 0xFFFFu) - 0x100u : (a & 0xFFFFu));
    uint32_t o;

    switch ((a >> 24) & 0xF) {
        case 0x2:
            return 0x02000000u | (a & 0x3FFFFu);
        case 0x3:
            return 0x03000000u | (a & 0x7FFFu);
        case 0x4:
            return (a & 0xFFFFFFu) < 0x400u ? 0x04000000u | (a & 0x3FFu) : bios;
        case 0x5:
            return 0x05000000u | (a & 0x3FFu);
        case 0x6:
            o = a & 0x1FFFFu;
            return 0x06000000u | (o >= 0x18000u ? o - 0x8000u : o);
        case 0x7:
            return 0x07000000u | (a & 0x3FFu);
        case 0x8: case 0x9: case 0xA: case 0xB: case 0xC: case 0xD:
            return write ? bios : 0x08000000u | (a & 0x1FFFFFFu);
        case 0xE: case 0xF:
            return 0x0E000000u | (a & 0xFFFFu);
        default:
            return bios;
    }
}

static int AddressRegister(const uint8_t* p, uint32_t shift, int* reg, uint32_t* delta) {
    int i;
    uint8_t op;

    for (i = 0; i < 15; i++, p++) {
        if (*p == 0x67)
            return 0; /* 16-bit addressing */
        if (*p != 0x66 && *p != 0xF2 && *p != 0xF3 && *p != 0xF0 && *p != 0x2E && *p != 0x3E && *p != 0x26
            && *p != 0x36 && *p != 0x64 && *p != 0x65)
            break;
    }
    op = *p++;
    /* String instructions: source ESI, destination EDI. */
    if ((op >= 0xA4 && op <= 0xA7) || (op >= 0xAA && op <= 0xAF)) {
        *reg = (op == 0xAA || op == 0xAB || op == 0xAE || op == 0xAF) ? 7 : 6;
        *delta = shift;
        return 1;
    }
    if (op == 0x0F) {
        op = *p++;
        if (op == 0x38 || op == 0x3A)
            p++;
        else if (NoModRm2(op))
            return 0;
    } else if (!HasModRm1(op)) {
        return 0;
    }
    {
        uint8_t modrm = *p++;
        int mod = modrm >> 6, rm = modrm & 7;
        int base = -1, index = -1, scale = 1;

        if (mod == 3)
            return 0;
        if (rm == 4) {
            uint8_t sib = *p++;
            scale = 1 << (sib >> 6);
            index = (sib >> 3) & 7;
            base = sib & 7;
            if (index == 4)
                index = -1;
            if (base == 5 && mod == 0)
                base = -1;
        } else if (!(rm == 5 && mod == 0)) {
            base = rm;
        }
        if (base >= 0 && base != index) {
            *reg = base;
            *delta = shift;
            return 1;
        }
        if (index >= 0 && base < 0 && shift % (uint32_t)scale == 0) {
            *reg = index;
            *delta = shift / (uint32_t)scale;
            return 1;
        }
    }
    return 0;
}

static void LogSite(const EXCEPTION_RECORD* r, uint32_t eip) {
    int i;

    for (i = 0; i < sLoggedCount; i++)
        if (sLogged[i] == eip)
            return;
    if (sLoggedCount < LOGGED_MAX)
        sLogged[sLoggedCount++] = eip;
    PortLog("bios: %s %08lx at eip %08lx", r->ExceptionInformation[0] ? "write" : "read",
            (unsigned long)r->ExceptionInformation[1], (unsigned long)eip);
}

static LONG CALLBACK BiosFaultHandler(EXCEPTION_POINTERS* info) {
    EXCEPTION_RECORD* r = info->ExceptionRecord;
    CONTEXT* c = info->ContextRecord;

    if (r->ExceptionCode == EXCEPTION_SINGLE_STEP) {
        if (sPending.reg < 0 || sPending.thread != GetCurrentThreadId())
            return EXCEPTION_CONTINUE_SEARCH;
        {
            DWORD* v = Reg(c, sPending.reg);
            /* Move it back unless the instruction loaded something new into it
               (it then no longer points near the redirected location). */
            if (*v - sPending.target + 0x10000u < 0x20000u)
                *v -= sPending.delta;
        }
        sPending.reg = -1;
        c->EFlags &= ~0x100u;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (r->ExceptionCode != EXCEPTION_ACCESS_VIOLATION || r->NumberParameters < 2 || r->ExceptionInformation[0] == 8
        || sPending.reg >= 0)
        return EXCEPTION_CONTINUE_SEARCH;
    {
        uint32_t fault = (uint32_t)r->ExceptionInformation[1];
        int reg;
        uint32_t delta;

        /* Only addresses outside the GBA space: anything else is a real fault. */
        if (fault >= GBA_SPACE_LOW && fault < GBA_SPACE_HIGH)
            return EXCEPTION_CONTINUE_SEARCH;
        if (fault >= BIOS_AREA_SIZE && fault < GBA_SPACE_LOW)
            return EXCEPTION_CONTINUE_SEARCH; /* Windows' own address range */
        if (!AddressRegister((const uint8_t*)(uintptr_t)c->Eip, BusTarget(fault, r->ExceptionInformation[0] == 1) - fault,
                             &reg, &delta))
            return EXCEPTION_CONTINUE_SEARCH;
        LogSite(r, c->Eip);
        *Reg(c, reg) += delta;
        sPending.reg = reg;
        sPending.delta = delta;
        sPending.target = *Reg(c, reg);
        sPending.thread = GetCurrentThreadId();
        c->EFlags |= 0x100u; /* trap after this one instruction */
        return EXCEPTION_CONTINUE_EXECUTION;
    }
}

void PortBiosFaultInstall(void) {
    AddVectoredExceptionHandler(1, BiosFaultHandler);
}
