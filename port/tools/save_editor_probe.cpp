// Command-line driver of port/decompui/save_editor.h for tests:
//   save_editor_probe <in.sav> <out.sav> <file 0-3> [level N] [hp N] [cp N]
//                     [moogle N] [map ID COUNT] [add ID COPIES] [remove ID COPIES]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include "../decompui/save_editor.h"

using namespace RekhcomSaveEdit;

int main(int argc, char** argv) {
    if (argc < 4) return 2;
    std::ifstream in(argv[1], std::ios::binary);
    std::vector<uint8_t> sram((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    File f;
    if (!Read(sram, std::atoi(argv[3]), f)) {
        std::printf("no save in file %s\n", argv[3]);
        return 1;
    }
    for (int i = 4; i < argc; i++) {
        const char* op = argv[i];
        int a = i + 1 < argc ? std::atoi(argv[i + 1]) : 0, b = i + 2 < argc ? std::atoi(argv[i + 2]) : 0;
        if (!std::strcmp(op, "level")) f.SetLevel(a), i++;
        else if (!std::strcmp(op, "hp")) f.SetMaxHp(a), i++;
        else if (!std::strcmp(op, "cp")) f.SetCp(a), i++;
        else if (!std::strcmp(op, "moogle")) f.SetMoogle(a), i++;
        else if (!std::strcmp(op, "map")) f.SetMapCard(a, b), i += 2;
        else if (!std::strcmp(op, "add")) std::printf("added %d\n", f.AddCard(a, b, false)), i += 2;
        else if (!std::strcmp(op, "remove")) std::printf("removed %d\n", f.RemoveCard(a, b)), i += 2;
    }
    Write(sram, f);
    std::printf("level %d hp %d cp %d exp %u moogle %u\n", f.Level(), f.MaxHp(), f.Cp(), f.Exp(), f.Moogle());
    std::ofstream out(argv[2], std::ios::binary);
    out.write((const char*)sram.data(), (std::streamsize)sram.size());
    return 0;
}
