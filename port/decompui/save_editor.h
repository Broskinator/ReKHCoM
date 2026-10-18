#ifndef REKHCOM_SAVE_EDITOR_H
#define REKHCOM_SAVE_EDITOR_H
/*
 * The launcher's save editor: reads one of the four files of a 64 KB save
 * (Sora 1-2, Riku 1-2), changes it and writes both of its copies with the
 * game's own checksum, plus the file's summary in the header (title screen).
 * Same layout in the three cartridges (include/save.h, save_types.h):
 *   header 0x10 (2 x 0x40), Riku 0x1EC0 (2 files x 2 copies x 0x418),
 *   Sora 0x2F20 (2 files x 2 copies x 0xF14); each block: 24-byte signature,
 *   u16 checksum, data at +0x1C.
 */
#include <cstdint>
#include <cstring>
#include <vector>

namespace RekhcomSaveEdit {

static const size_t kSramSize = 0x10000;
static const size_t kHeader = 0x10, kHeaderSize = 0x40;
static const size_t kRiku = 0x1EC0, kRikuSize = 0x418;
static const size_t kSora = 0x2F20, kSoraSize = 0xF14;
static const size_t kData = 0x1C;
/* SaveCommon (data +0): flags, then the player's progression */
static const size_t kFlags = 0x00, kProgress = 0x04, kHp = 0x8E, kFloor = 0x90, kWorld = 0x91, kPlayTime = 0x94;
/* PlayerProgression (+kProgress) */
static const size_t kMaxHp = 0x00, kCp = 0x02, kDp = 0x04, kAp = 0x06, kExp = 0x08, kNextExp = 0x0C, kLevel = 0x10,
                    kMoogle = 0x7C, kMilestone = 0x80;
/* the card slices after MapProgress (data +0x2EC) */
static const size_t kSlice = 0x2EC, kMapCards = 0x2ED /* Sora */, kRikuMapCards = 0x2EC, kMapCardCount = 0x10E;
static const size_t kCards = 0x2EC + 0x110, kCardCount = 0x2EC + 0x8DE, kDecks = 0x2EC + 0x8E0;
static const int kCollection = 999;
static const uint16_t kEmpty = 0xFFF, kIdMask = 0xFFF, kInDeck = 0x7000;

inline uint16_t Get16(const uint8_t* p) { return (uint16_t)(p[0] | p[1] << 8); }
inline uint32_t Get32(const uint8_t* p) { return Get16(p) | (uint32_t)Get16(p + 2) << 16; }
inline void Put16(uint8_t* p, unsigned v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
inline void Put32(uint8_t* p, uint32_t v) { Put16(p, v & 0xFFFF); Put16(p + 2, v >> 16); }

/* SaveChecksum (src/save.c): one's complement of the 16-bit word sum */
inline uint16_t Checksum(const uint8_t* block, size_t size) {
    uint32_t sum = 0;
    for (size_t i = 0; i + 1 < size; i += 2)
        sum += Get16(block + i);
    if (size & 1)
        sum += block[size - 1];
    sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~(sum + (sum >> 16));
}

inline bool BlockValid(const uint8_t* block, size_t size) {
    static const char kSignature[] = "KHCOM_BACKUP_VER000000"; /* gSaveSignatureText: ...13 EU, ...12 US/JP */
    return std::memcmp(block, kSignature, sizeof(kSignature) - 1) == 0 && Checksum(block, size) == 0;
}

inline void Seal(uint8_t* block, size_t size) {
    Put16(block + 24, 0);
    Put16(block + 24, Checksum(block, size));
}

/* file: 0-1 Sora, 2-3 Riku (the game's order) */
inline size_t BlockSize(int file) { return file < 2 ? kSoraSize : kRikuSize; }
inline size_t BlockAt(int file, int copy) {
    return file < 2 ? kSora + (size_t)file * 2 * kSoraSize + copy * kSoraSize
                    : kRiku + (size_t)(file - 2) * 2 * kRikuSize + copy * kRikuSize;
}

struct File {
    int file = -1;
    std::vector<uint8_t> block; /* the valid copy, signature to end */
    uint8_t* data() { return block.data() + kData; }
    const uint8_t* data() const { return block.data() + kData; }
    bool riku() const { return file >= 2; }

    int Level() const { return data()[kProgress + kLevel]; }
    int MaxHp() const { return (int16_t)Get16(data() + kProgress + kMaxHp); }
    int Cp() const { return (int16_t)Get16(data() + kProgress + kCp); }
    int Ap() const { return (int16_t)Get16(data() + kProgress + kAp); }
    int Dp() const { return (int16_t)Get16(data() + kProgress + kDp); }
    uint32_t Exp() const { return Get32(data() + kProgress + kExp); }
    uint32_t Moogle() const { return Get32(data() + kProgress + kMoogle); }
    uint32_t PlayTime() const { return Get32(data() + kPlayTime); }
    int Floor() const { return data()[kFloor]; }
    int World() const { return data()[kWorld]; }
    uint8_t* MapCards() { return data() + (riku() ? kRikuMapCards : kMapCards); }
    const uint8_t* MapCards() const { return data() + (riku() ? kRikuMapCards : kMapCards); }
    uint16_t Card(int i) const { return Get16(data() + kCards + 2 * i); }
    int Capacity() const { return Get16(data() + kCardCount); }

    /* level L with the experience it takes (AdvanceLevelExpThreshold) */
    void SetLevel(int level) {
        uint32_t next = 0x19, reached = 0;
        if (level < 1) level = 1;
        if (level > 99) level = 99;
        for (int l = 1; l < level; l++) {
            reached = next;
            next += (uint32_t)(l + 1) * (l + 1) * 3;
        }
        data()[kProgress + kLevel] = (uint8_t)level;
        Put32(data() + kProgress + kNextExp, next);
        if (Exp() < reached || Exp() >= next)
            Put32(data() + kProgress + kExp, reached);
    }
    void SetMaxHp(int v) { Put16(data() + kProgress + kMaxHp, v < 1 ? 1 : v > 560 ? 560 : v); }
    void SetCp(int v) { Put16(data() + kProgress + kCp, v < 0 ? 0 : v > 9999 ? 9999 : v); }
    void SetAp(int v) { Put16(data() + kProgress + kAp, v < 0 ? 0 : v > 999 ? 999 : v); }
    void SetDp(int v) { Put16(data() + kProgress + kDp, v < 0 ? 0 : v > 999 ? 999 : v); }
    void SetMoogle(uint32_t v) { Put32(data() + kProgress + kMoogle, v > 9999999 ? 9999999 : v); }
    void SetMapCard(int id, int count) { MapCards()[id] = (uint8_t)(count < 0 ? 0 : count > 9 ? 9 : count); }

    int CountCard(int id) const {
        int n = 0;
        for (int i = 0; i < kCollection; i++)
            n += Card(i) != kEmpty && (Card(i) & kIdMask) == id;
        return n;
    }
    /* adds copies of a card into free slots (99 per card, 999 in all, as
       AddCardToCollection); returns how many were added */
    int AddCard(int id, int copies, bool premium) {
        int added = 0;
        if (Capacity() < kCollection)
            Put16(data() + kCardCount, kCollection);
        for (int i = 0; i < kCollection && added < copies && CountCard(id) < 99; i++) {
            if (Card(i) != kEmpty) continue;
            Put16(data() + kCards + 2 * i, (unsigned)id | (premium ? 0x8000u : 0));
            added++;
        }
        return added;
    }
    /* removes copies not placed in any deck; returns how many */
    int RemoveCard(int id, int copies) {
        int removed = 0;
        for (int i = kCollection - 1; i >= 0 && removed < copies; i--) {
            uint16_t c = Card(i);
            if (c == kEmpty || (c & kIdMask) != id || (c & kInDeck)) continue;
            Put16(data() + kCards + 2 * i, kEmpty);
            removed++;
        }
        return removed;
    }
};

/* the file's valid copy; false when neither copy holds a save */
inline bool Read(const std::vector<uint8_t>& sram, int file, File& out) {
    if (sram.size() != kSramSize || file < 0 || file > 3) return false;
    for (int copy = 0; copy < 2; copy++) {
        const uint8_t* b = sram.data() + BlockAt(file, copy);
        if (BlockValid(b, BlockSize(file))) {
            out.file = file;
            out.block.assign(b, b + BlockSize(file));
            return true;
        }
    }
    return false;
}

/* both copies of the file, then its summary in both header copies */
inline void Write(std::vector<uint8_t>& sram, File& f) {
    Seal(f.block.data(), f.block.size());
    for (int copy = 0; copy < 2; copy++)
        std::memcpy(sram.data() + BlockAt(f.file, copy), f.block.data(), f.block.size());
    for (int copy = 0; copy < 2; copy++) {
        uint8_t* h = sram.data() + kHeader + copy * kHeaderSize;
        if (!BlockValid(h, kHeaderSize)) continue;
        uint8_t* summary = h + kData + 4 + f.file * 8;
        summary[0] = (uint8_t)f.Floor();
        summary[1] = (uint8_t)f.World();
        summary[2] = (uint8_t)f.Level();
        Put32(summary + 4, f.PlayTime());
        Seal(h, kHeaderSize);
    }
}

}
#endif
