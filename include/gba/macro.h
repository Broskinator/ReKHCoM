#ifndef GUARD_GBA_MACRO_H
#define GUARD_GBA_MACRO_H

#include "gba/io_reg.h"
#include "gba/syscall.h"
#include "types.h"

#define CpuCopy16(src, dest, size) CpuSet(src, dest, ((size) / 2) & 0x1FFFFF)
#define CpuCopy32(src, dest, size) CpuSet(src, dest, CPU_SET_32BIT | (((size) / 4) & 0x1FFFFF))
#define CpuFill16(value, dest, size)                                                          \
{                                                                                             \
    vu16 tmp = (vu16)(value);                                                                 \
    CpuSet((void*)&tmp, dest, CPU_SET_SRC_FIXED | (((size) / 2) & 0x1FFFFF));                 \
}
#define CpuFill32(value, dest, size)                                                          \
{                                                                                             \
    vu32 tmp = (vu32)(value);                                                                 \
    CpuSet((void*)&tmp, dest, CPU_SET_32BIT | CPU_SET_SRC_FIXED | (((size) / 4) & 0x1FFFFF)); \
}

#define CpuFastCopy(src, dest, size) CpuFastSet(src, dest, ((size) / 4) & 0x1FFFFF)
#define CpuFastFill(value, dest, size)                                                        \
{                                                                                             \
    vu32 tmp = (vu32)(value);                                                                 \
    CpuFastSet((void*)&tmp, dest, CPU_FAST_SET_SRC_FIXED | (((size) / 4) & 0x1FFFFF));        \
}

#ifdef PLATFORM_PC
void PortDmaSet(u32 dmaNum, const void* src, void* dest, u32 control);
#define DmaSet(dmaNum, src, dest, control) PortDmaSet(dmaNum, (const void*)(src), (void*)(dest), (u32)(control))
#else
#define DmaSet(dmaNum, src, dest, control)       \
{                                                \
    vu32* dmaRegs = (vu32*)REG_ADDR_DMA##dmaNum; \
    dmaRegs[0] = (vu32)(src);                    \
    dmaRegs[1] = (vu32)(dest);                   \
    dmaRegs[2] = (vu32)(control);                \
    dmaRegs[2];                                  \
}
#endif

#define DmaStop(dmaNum)                                         \
{                                                               \
    vu16* dmaRegs = (vu16*)REG_ADDR_DMA##dmaNum;                \
    dmaRegs[5] &= ~(DMA_START_MASK | DMA_DREQ_ON | DMA_REPEAT); \
    dmaRegs[5] &= ~DMA_ENABLE;                                  \
    dmaRegs[5];                                                 \
}

#define DmaCopy16(dmaNum, src, dest, size) \
    DmaSet(dmaNum, src, dest, (DMA_ENABLE << 16) | ((size) / 2))
#define DmaCopy32(dmaNum, src, dest, size) \
    DmaSet(dmaNum, src, dest, ((DMA_ENABLE | DMA_32BIT) << 16) | ((size) / 4))
#define DmaFill16(dmaNum, value, dest, size)                                         \
{                                                                                    \
    vu16 tmp = (vu16)(value);                                                        \
    DmaSet(dmaNum, &tmp, dest, ((DMA_ENABLE | DMA_SRC_FIXED) << 16) | ((size) / 2)); \
}
#define DmaFill32(dmaNum, value, dest, size)                                                     \
{                                                                                                \
    vu32 tmp = (vu32)(value);                                                                    \
    DmaSet(dmaNum, &tmp, dest, ((DMA_ENABLE | DMA_32BIT | DMA_SRC_FIXED) << 16) | ((size) / 4)); \
}

#endif
