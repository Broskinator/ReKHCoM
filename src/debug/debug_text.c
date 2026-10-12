/**
 * debug_text.c
 * Debug Text Printing
 */

#include "debug_font.h"
#include "malloc.h"
#include "display.h"
#include "gba/defines.h"
#include "types.h"
#include "gba/macro.h"
#include <stddef.h>
#include "debug_text.h"
#include "macros.h"

static u8* sDebugTextTileDest;
static u8 sDebugTextPaletteBank;
static DebugTextLine* sDebugTextLines;
static u8 sDebugTextLineCount;
static u8 sUnk_02034A21;
static s32 sUnk_02034A24;
static s32 sDebugTextMergeFirstGlyph;
static void* sUnk_02034A2C;

void DebugTextClearBg() {
    void* charBase = GetBgCharBase(0);
    void* screenBase = GetBgScreenBase(0);

    CpuFastFill(0, charBase, 0x5400);
    CpuFastFill(0, screenBase, 0x500);
}

void func_0805F7B0(s32 a) {
    sUnk_02034A24 = a;
}

void DebugTextClearScreen() {
    DebugTextClearBg();
}

void func_0805F7C8(u8 block) {
    sUnk_02034A2C = (u8*)GetBgCharBase(0) + (block << 12);
}

void DebugTextPrintFont2(u8 x, u8 y, u16* text) {
    u8 i;
    u16 character;

    for (i = 0; i <= 59 && (u8)*text != 0; i++, text++) {
        character = *text;
        character = (u8)(character >> 8) | (character << 8);

        switch (character & 0xFF00) {
        case 0x8100:
            sDebugTextLines[sDebugTextLineCount].glyphs[i] = character + 0x7EC0;
            break;
        case 0x8200:
            sDebugTextLines[sDebugTextLineCount].glyphs[i] = (character + 0x7DC0) | 0x400;
            break;
        }
    }

    sDebugTextLines[sDebugTextLineCount].x = x;
    sDebugTextLines[sDebugTextLineCount].y = y;
    sDebugTextLines[sDebugTextLineCount].length = i;
    sDebugTextLineCount++;
}

u8 DebugTextGetPixelShift(u8 x) {
    return x * 4 % 32;
}

void DebugTextClearLines() {
    u8 i;
    u8 j;

    for (i = 0; i <= 19; i++) {
        for (j = 0; j < ARRAY_COUNT(sDebugTextLines[i].glyphs); j++) {
            sDebugTextLines[i].glyphs[j] = 0;
        }
    }
}

s32 DebugTextScrollUp(u8 bg, u8 x, u8 y, u8 width, u8 height) {
    u8 i;
    u8 j;
    u8 row;
    u8 pixelRow;
    u16 col;
    u8* tile;
    u32* dst;
    u32* src;
    u32 nextOffset;
    u32 rowOffset;
    u32 nextRowOffset;
    u32 co;
    s32 n;

    j = 0;
    sDebugTextTileDest = (u8*)GetBgCharBase(bg) + y * 0x400 + (x + 1) * 32;
    x = ((x + width) >> 3) + 1;
    n = (s8)width + x;
    width = n;
    height += (y + height) >> 3;
    pixelRow = y & 7;
    col = (y >> 3) << 3;

    for (; j < height; j++) {
        for (i = 0; i < width; i++) {
            rowOffset = pixelRow * 4;
            co = col * 4;
            nextRowOffset = rowOffset + 4;
            tile = sDebugTextTileDest + i * 32 + j * 1024;
            dst = (u32*)(tile + rowOffset + co);
            src = (u32*)(tile + nextRowOffset + co);

            for (row = 0; row < 9; row++) {
                nextOffset = ((u32)dst & 0xFF) + 4;

                if (nextOffset == ((nextOffset >> 5) << 5)) {
                    *dst++ = *(u32*)((u8*)src + 0x3E0);
                    dst += 248;
                    src += 249;
                } else {
                    *dst++ = *src++;
                }
            }
        }

        pixelRow++;

        if (pixelRow == 8) {
            col += 0x100;
            pixelRow = 0;
        }
    }

    return 1;
}

void DebugTextLoadPalette(s32 bg, const void* palette, s32 size, u8 bank) {
    if (palette != NULL) {
        LoadPalette(palette, (void*)(bank * 32 + PLTT), 32);
    }

    sDebugTextPaletteBank = bank;
}

void DebugTextInit(u8 bg, u16 charSize, u16 screenSize) {
    u8 i;
    u8 j;
    void* charBase = GetBgCharBase(bg);
    void* screenBase = GetBgScreenBase(bg);

    CpuFill32(0, charBase, charSize);
    CpuFill32(0, screenBase, screenSize);

    // @bug Only 20 lines are allocated, but OPE SAVE prints up to 40.
    sDebugTextLines = EwramAlloc(sizeof(DebugTextLine) * 20);
    sDebugTextLineCount = 0;

    for (i = 0; i <= 19; i++) {
        for (j = 0; j < ARRAY_COUNT(sDebugTextLines[i].glyphs); j++) {
            sDebugTextLines[i].glyphs[j] = 0;
        }

        sDebugTextLines[i].x = 0;
        sDebugTextLines[i].y = 0;
        sDebugTextLines[i].unk_7C = 0;
        sDebugTextLines[i].length = 0;
    }

    func_0805F7B0(0);
    sUnk_02034A21 = 0;
    sDebugTextPaletteBank = 0;
    EnableBg(bg);
}

void DebugTextSetMergeFirstGlyph(s32 on) {
    sDebugTextMergeFirstGlyph = on;
}

void DebugTextPrintXNumber(u8 x, u8 y, u32 font, u8 value) {
    u8 buf[8];

    buf[3] = value / 10;
    buf[5] = value - buf[3] * 10;
    buf[0] = 0x82;
    buf[1] = 0x98;
    buf[2] = 0x82;
    buf[3] += 0x4F;
    buf[4] = 0x82;
    buf[5] += 0x4F;
    buf[6] = 0;
    DebugTextPrint(x, y, font, buf);
}

void DebugTextPrintNumber(u8 x, u8 y, u32 font, u16 value) {
    u8 buf[8];

    buf[1] = value / 100;
    buf[3] = value / 10 - buf[1] * 10;
    buf[5] = value - (buf[1] * 100 + buf[3] * 10);
    buf[0] = 0x82;
    buf[1] += 0x4F;
    buf[2] = 0x82;
    buf[3] += 0x4F;
    buf[4] = 0x82;
    buf[5] += 0x4F;
    buf[6] = 0;
    DebugTextPrint(x, y, font, buf);
}

void DebugTextPrint(u8 x, u8 y, u32 font, const char* text) {
    u8 i = 0;
    s32 shift = 0;
    u16 character;

    switch (font) {
    case 0:
        shift = 1;
        break;
    case 1:
        shift = 0;
        break;
    case 2:
        sDebugTextLines[sDebugTextLineCount].font = font;
        DebugTextPrintFont2(x, y, (u16*)text);
        return;
    }

    if (sDebugTextLineCount > 19) {
        return;
    }

    while (*text != 0) {
        character = *(const u16*)text;
        character = (character >> 8) | (character << 8);

        switch (character & 0xFF00) {
        case 0x8100:
            if (character > 0x8146) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8146 + (0x42 >> shift);
            }

            switch (character) {
            case 0x8140:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 0;
                break;
            case 0x815E:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 1;
                break;
            case 0x815B:
            case 0x815C:
            case 0x815D:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 2;
                break;
            case 0x8151:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 3;
                break;
            case 0x8144:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 4;
                break;
            case 0x817B:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 5;
                break;
            case 0x8149:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 6;
                break;
            case 0x8148:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 7;
                break;
            case 0x8194:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 8;
                break;
            case 0x8193:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 9;
                break;
            case 0x818D:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 10;
                break;
            case 0x818B:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 11;
                break;
            case 0x8196:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 12;
                break;
            case 0x8168:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 13;
                break;
            case 0x8190:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 14;
                break;
            case 0x8195:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 15;
                break;
            case 0x8166:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 16;
                break;
            case 0x8169:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 17;
                break;
            case 0x816A:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 18;
                break;
            case 0x8181:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 19;
                break;
            case 0x8160:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 20;
                break;
            case 0x8162:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 21;
                break;
            case 0x8197:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 22;
                break;
            case 0x8165:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 23;
                break;
            case 0x8175:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 24;
                break;
            case 0x8176:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 25;
                break;
            case 0x816F:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 26;
                break;
            case 0x8170:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 27;
                break;
            case 0x8141:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 28;
                break;
            case 0x8142:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 29;
                break;
            case 0x8183:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 30;
                break;
            case 0x8184:
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = 31;
                break;
            }

            break;
        case 0x8200:
            if ((u16)(character - 0x824F) <= 9) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x824F + (0x80 >> shift);
            }

            if ((u16)(character - 0x8260) <= 25) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8260 + (0xC0 >> shift);
            }

            if ((u16)(character - 0x8281) <= 25) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8281 + (0x100 >> shift);
            }

            if ((u16)(character - 0x829F) <= 31) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x829F + (0x140 >> shift);
            }

            if ((u16)(character - 0x82BF) <= 31) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x82BF + (0x180 >> shift);
            }

            if ((u16)(character - 0x82DF) <= 31) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x82DF + (0x1C0 >> shift);
            }

            break;
        case 0x8300:
            if ((u16)(character - 0x8340) <= 31) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8340 + ((0x200 - shift * 192) >> shift);
            }

            if ((u16)(character - 0x8360) <= 30) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8360 + ((0x240 - shift * 192) >> shift);
            }

            if (character == 0x8380) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = ((0x25F - shift * 192) >> shift);
            }

            if ((u16)(character - 0x8381) <= 21) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x8381 + ((0x280 - shift * 192) >> shift);
            }

            if ((u16)(character - 0x83BF) <= 1) {
                sDebugTextLines[sDebugTextLineCount].glyphs[i] = character - 0x83BF + (0x40 >> shift);
            }

            break;
        }

        i++;
        text += 2;

        if (i > 59) {
            break;
        }
    }

    sDebugTextLines[sDebugTextLineCount].x = x;
    sDebugTextLines[sDebugTextLineCount].y = y;
    sDebugTextLines[sDebugTextLineCount].length = i;
    sDebugTextLines[sDebugTextLineCount].font = font;
    sDebugTextLineCount++;
}

void DebugTextDrawAligned(u8 bg) {
    u8 n;
    u8 i;
    u8 k;
    u8 x;
    u8 y;
    u16* screen;
    u32* tiles;
    u32* src;

    screen = GetBgScreenBase(bg);

    for (n = 0; n < sDebugTextLineCount; n++) {
        tiles = (u32*)((u8*)GetBgCharBase(bg) + (n * 0x1000 + 0x2000));
        x = sDebugTextLines[n].x;
        y = sDebugTextLines[n].y;

        for (i = 0; i < sDebugTextLines[n].length; i++) {
            src = (u32*)&gDebugFont1Tiles[sDebugTextLines[n].glyphs[i] * 32];

            for (k = 0; k < 8; k++) {
                tiles[k] = src[0];
                tiles[k + 0x100] = src[0x100];
                src++;
            }

            *(screen + x + i + (y << 5)) = (i + 0x100 + n * 128) | 0x1000;
            *(screen + x + i + ((y + 1) << 5)) = (i + 0x120 + n * 128) | 0x1000;
            tiles += 8;
        }
    }
}

void DebugTextClear() {
    sDebugTextLineCount = 0;
}

void DebugTextDraw(u8 bg) {
    void* charBase;
    u32 pixels;
    u32 mapRow;
    u8* screen;
    u8* font = NULL;
    u8 n;
    u8 tileX;
    u8 tileY;
    u8 offsetX;
    u8 offsetY;
    s32 height = 0;
    u8* destination;
    u8 i;
    u8 row;
    u8 sourceRow;

    charBase = GetBgCharBase(bg);
    sDebugTextTileDest = charBase;
    screen = GetBgScreenBase(bg);

    for (n = 0; n < sDebugTextLineCount; n++) {
        tileX = sDebugTextLines[n].x >> 3;
        tileY = sDebugTextLines[n].y >> 3;
        offsetX = sDebugTextLines[n].x - tileX * 8;
        offsetY = sDebugTextLines[n].y - tileY * 8;
        sDebugTextTileDest = (u8*)GetBgCharBase(bg) + (tileX + 1 + tileY * 32) * 32;

        for (i = 0; i < sDebugTextLines[n].length; i++) {
            destination = sDebugTextTileDest + i * 32;

            switch (sDebugTextLines[n].font) {
            case 0:
                font = gDebugFont0Tiles + sDebugTextLines[n].glyphs[i] * 32;
                height = 8;
                break;
            case 1:
                font = gDebugFont1Tiles + sDebugTextLines[n].glyphs[i] * 32;
                height = 10;
                break;
            case 2:
                font = gDebugFont2Banks[sDebugTextLines[n].glyphs[i] >> 10];
                height = 8;
                break;
            }

            for (row = offsetY, sourceRow = 0; row < offsetY + height; row++, sourceRow++) {
                if (offsetX == 0) {
                    if (sDebugTextLines[n].font != 2) {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sourceRow >> 3) * 0x400);
                    } else {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sDebugTextLines[n].glyphs[i] & 0x3FF) * 32);
                    }
                } else if (i != 0 || sDebugTextMergeFirstGlyph == TRUE) {
                    pixels = ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0];

                    if (sDebugTextLines[n].font != 2) {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = pixels | *(u32*)(font + (sourceRow & 7) * 4 + (sourceRow >> 3) * 0x400) << (offsetX * 4);
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))[1].rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sourceRow >> 3) * 0x400) >> (32 - offsetX * 4);
                    } else {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = pixels | *(u32*)(font + (sourceRow & 7) * 4 + (sDebugTextLines[n].glyphs[i] & 0x3FF) * 32) << (offsetX * 4);
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))[1].rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sDebugTextLines[n].glyphs[i] & 0x3FF) * 32) >> (32 - offsetX * 4);
                    }
                } else {
                    if (sDebugTextLines[n].font != 2) {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sourceRow >> 3) * 0x400) << (offsetX * 4);
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))[1].rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sourceRow >> 3) * 0x400) >> (32 - offsetX * 4);
                    } else {
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))->rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sDebugTextLines[n].glyphs[i] & 0x3FF) * 32) << (offsetX * 4);
                        ((CharTile*)(destination + (row & 7) * 4 + (row >> 3) * 0x400))[1].rows[0] = *(u32*)(font + (sourceRow & 7) * 4 + (sDebugTextLines[n].glyphs[i] & 0x3FF) * 32) >> (32 - offsetX * 4);
                    }
                }

                mapRow = (tileY + (u8)(row >> 3)) * 32;
                *(u16*)(screen + tileX * 2 + i * 2 + mapRow * 2) = (tileX + 1 + i + mapRow) | (sDebugTextPaletteBank << 12);
                *(u16*)(screen + tileX * 2 + i * 2 + mapRow * 2 + 2) = (tileX + 2 + i + mapRow) | (sDebugTextPaletteBank << 12);
            }
        }
    }

    DebugTextClearLines();
}

void DebugTextFree() {
    EwramFree(sDebugTextLines);
}

void DebugTextDestroy() {
    DebugTextFree();
}

u8* gDebugFont2Banks[2] = { gDebugFont2Bank0Tiles, gDebugFont2Bank1Tiles };
