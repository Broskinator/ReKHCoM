/**
 * bg_anim.c
 * Background Animation Player
 */

#include "pallet.h"
#include "gba/syscall.h"
#include <stddef.h>
#include "bg_animation_types.h"
#include "display.h"
#include "types.h"
#include "engine_math.h"
#include "gba/io_reg.h"

static BgAnimationDef* sBgAnimCurrent;
static u16 sBgAnimFrameTimer;
static u16 sBgAnimFrame;
static s32 sBgAnimBg;
static u16 sBgAnimFrameBytes;
static u16 sBgAnimFramesPerChunk;
static s16 sBgAnimScrollX;
static s16 sBgAnimScrollY;
static u8 sBgAnimStopped;
static u16 sBgAnimMapSize;
static u8 sBgAnimAffine;
static u32 sBgAnimScaleX;
static u32 sBgAnimScaleY;
static u8 sBgAnimRotation;
static s16 sBgAnimLoopStartFrame;
static s16 sBgAnimStopFrame;
static u16 sBgAnimFrameDuration;

void BgAnimInit(s32 bg, u16 size, u16 colorMode) {
    sBgAnimBg = bg;
    sBgAnimCurrent = NULL;
    sBgAnimScrollX = 0;
    sBgAnimScrollY = 0;
    sBgAnimStopped = TRUE;

    if (colorMode == BGCNT_16COLOR) {
        sBgAnimAffine = FALSE;

        switch (size) {
        case BGCNT_TXT512x256:
        case BGCNT_TXT256x512:
            sBgAnimMapSize = 0x1000;
            break;
        case BGCNT_TXT512x512:
            sBgAnimMapSize = 0x2000;
            break;
        case BGCNT_TXT256x256:
        default:
            sBgAnimMapSize = 0x800;
            break;
        }
    } else {
        sBgAnimAffine = TRUE;

        switch (size) {
        case BGCNT_AFF256x256:
            sBgAnimMapSize = 0x400;
            break;
        case BGCNT_AFF512x512:
            sBgAnimMapSize = 0x1000;
            break;
        case BGCNT_AFF1024x1024:
            sBgAnimMapSize = 0x4000;
            break;
        case BGCNT_AFF128x128:
        default:
            sBgAnimMapSize = 0x100;
            break;
        }
    }

    SetBgSize(bg, size);
    DisableBg(bg);
}

void BgAnimSetPosition(s16 x, s16 y) {
    if (sBgAnimAffine) {
        sBgAnimScrollX = -x;
        sBgAnimScrollY = -y;
    } else {
        sBgAnimScrollX = (sBgAnimCurrent->originX << 2) - x;
        sBgAnimScrollY = (sBgAnimCurrent->originY << 2) - y;
    }
}

void BgAnimSetTransform(u8 rotation, s32 scaleX, s32 scaleY) {
    sBgAnimRotation = rotation;
    sBgAnimScaleX = scaleX;
    sBgAnimScaleY = scaleY;
}

void BgAnimStart(BgAnimationDef* def, s32 x, s32 y) {
    sBgAnimCurrent = def;
    BgAnimSetPosition(x, y);

    if (sBgAnimAffine) {
        sBgAnimFrameBytes = def->tilesPerFrame << 6;
    } else {
        sBgAnimFrameBytes = def->tilesPerFrame << 5;
    }

    sBgAnimFramesPerChunk = 0x8000 / sBgAnimFrameBytes;
    sBgAnimLoopStartFrame = -1;
    sBgAnimStopFrame = -1;
    sBgAnimFrameTimer = 0;
    sBgAnimFrame = 0;
    sBgAnimStopped = FALSE;
    sBgAnimFrameDuration = def->frameDuration;

    if (sBgAnimAffine) {
        sBgAnimScaleX = Q_8_8(1);
        sBgAnimScaleY = Q_8_8(1);
        sBgAnimRotation = 0;
    }

    PushPaletteEffect(0);
    LoadBgPalette(sBgAnimBg, def->palette, def->paletteSize);
    PopPaletteEffect();
    LoadBgMap(sBgAnimBg, def->tilemap, sBgAnimMapSize);
}

void BgAnimApplyAffineTransform(s32 bg, u8 rot, s32 sx, s32 sy, s16 cx, s16 cy) {
    BgAffineSrcData src;
    BgAffineDstData dst;

    src.texX = sBgAnimCurrent->originX << 10;
    src.texY = sBgAnimCurrent->originY << 10;
    src.scrX = -cx;
    src.scrY = -cy;
    src.sx = 0x10000 / sx;
    src.sy = 0x10000 / sy;
    src.alpha = -rot << 8;
    BgAffineSet(&src, &dst, 1);

    switch (bg) {
    case 2:
        gBg2PA = dst.pa;
        gBg2PB = dst.pb;
        gBg2PC = dst.pc;
        gBg2PD = dst.pd;
        gBg2X = dst.dx;
        gBg2Y = dst.dy;
        break;
    case 3:
        gBg3PA = dst.pa;
        gBg3PB = dst.pb;
        gBg3PC = dst.pc;
        gBg3PD = dst.pd;
        gBg3X = dst.dx;
        gBg3Y = dst.dy;
        break;
    }
}

void BgAnimUpdate() {
    u8* src;
    u16 chunk;
    u16 off;
    u16 len;
    s16 over;
    s32 vis;

    if (sBgAnimCurrent == NULL) {
        return;
    }

    if (sBgAnimFrame >= sBgAnimCurrent->frameCount) {
        if (sBgAnimLoopStartFrame >= 0) {
            sBgAnimFrame = sBgAnimLoopStartFrame;
            sBgAnimFrameTimer = 0;
        } else {
            BgAnimStop();
        }

        return;
    }

    if (sBgAnimAffine) {
        BgAnimApplyAffineTransform(sBgAnimBg, sBgAnimRotation, sBgAnimScaleX, sBgAnimScaleY, sBgAnimScrollX, sBgAnimScrollY);
        vis = TRUE;
    } else {
        SetBgScroll(sBgAnimBg, (u16)sBgAnimScrollX, (u16)sBgAnimScrollY);

        if (sBgAnimScrollX > -256 && sBgAnimScrollX < 128 && sBgAnimScrollY < 128 && sBgAnimScrollY > -256) {
            vis = TRUE;
        } else {
            vis = FALSE;
        }
    }

    if (vis) {
        EnableBg(sBgAnimBg);

        if (sBgAnimFrameTimer == 0) {
            chunk = sBgAnimFrame / sBgAnimFramesPerChunk;
            off = sBgAnimFrame % sBgAnimFramesPerChunk * sBgAnimFrameBytes;
            // @bug A frame past the last chunk has no data (NULL read).
            src = (u8*)sBgAnimCurrent->chunks[chunk].data + off;
            over = off + sBgAnimFrameBytes - sBgAnimCurrent->chunks[chunk].size;

            if (over > 0) {
                len = sBgAnimFrameBytes - over;
                RequestDma3Copy(src, GetBgCharBase(sBgAnimBg), len);
                RequestDma3Clear((u8*)GetBgCharBase(sBgAnimBg) + len, over);
            } else {
                RequestDma3Copy(src, GetBgCharBase(sBgAnimBg), sBgAnimFrameBytes);
            }
        }
    } else {
        DisableBg(sBgAnimBg);
    }

    sBgAnimFrameTimer++;

    if (sBgAnimFrameTimer >= sBgAnimFrameDuration) {
        sBgAnimFrameTimer = 0;

        if (sBgAnimFrame != sBgAnimStopFrame) {
            sBgAnimFrame++;
        }
    }
}

void BgAnimSetFrameDuration(u16 duration) {
    sBgAnimFrameDuration = duration;
}

void BgAnimSetLoopStartFrame(u16 frame) {
    sBgAnimLoopStartFrame = frame;
}

void BgAnimSetStopFrame(u16 frame) {
    sBgAnimStopFrame = frame;
}

void BgAnimStop() {
    sBgAnimCurrent = NULL;
    sBgAnimStopped = TRUE;
    DisableBg(sBgAnimBg);
}

u8 BgAnimIsStopped() {
    return sBgAnimStopped;
}

void BgAnimGetFrameState(u16* frame, u16* timer) {
    if (frame != NULL) {
        *frame = sBgAnimFrame;
    }

    if (timer != NULL) {
        *timer = sBgAnimFrameTimer;
    }
}

u32 BgAnimGetDuration(BgAnimationDef* def) {
    return (u32)def->frameCount * def->frameDuration;
}

BgAnimationDef* BgAnimGetCurrent() {
    return sBgAnimCurrent;
}
