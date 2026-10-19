"""Exercise production preview bodies and cheat eligibility for issue #21.
ROM-free, sequential native checks; no player save, game or account is opened.
"""
from pathlib import Path
import json
import re
import subprocess
import sys
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'build/issue21'; OUT.mkdir(parents=True, exist_ok=True)
CC = next((ROOT.parent / 'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
def function(path, name):
    source = (ROOT / path).read_text(encoding='utf-8')
    match = re.search(r'(?:static )?(?:void|int) ' + name + r'\([^)]*\)\s*\{', source)
    assert match, name
    end = match.end(); depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}'); end += 1
    return source[match.start():end]
source = (ROOT / 'port/web/cards.js').read_text(encoding='utf-8')
cards = json.loads(source[source.index('={')+1:].rstrip().rstrip(';'))['cards']
source = (ROOT / 'src/card/mode_deckexchange.c').read_text(encoding='utf-8')
table = source.split('CardDescriptionText* gCardKindDescriptions[98] = {',1)[1].split('};',1)[0]
entries = re.findall(r'#if defined\(VERSION_EU\)\s+([^#]+).*?#endif|\b(NULL),',table,re.S)
assert len(entries) == 98 and [i for i, (_, n) in enumerate(entries) if n] == [95]
body = '\n'.join(function(p,n) for p,n in [
 ('port/src/port_cheats.c','CollectableCard'),
 ('src/card/card_deckmenu2.c','LoadCardDescriptionText'),
 ('src/card/card_deckmenu2_riku.c','LoadRikuCardDescriptionText'),
 ('src/card/card_deckexchange.c','LoadDeckExchangeCardDescriptionText')])
header = r"""
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "macros.h"
#include "system_state.h"
#include "card.h"
#include "card_ids.h"
#include "card_def_data.h"
#include "text.h"
u32 gLanguage;
static u8 testText[2];
#ifdef VERSION_EU
static LocalizedText testDescription = {{testText,testText,testText,testText,testText}};
#else
#define testDescription (*(CardDescriptionText*)testText)
#endif
static int loads;
u16 LoadTextSlots(const void* text, TextSlot* slots) { assert(text && slots); ++loads; return 7; }
"""
header += 'const CardDef gCardDefs[950] = {\n' + ''.join(
 '    {.kind=%d,.flags=%d,.value=%d,.category=%d},\n' % (c[0],c[1],c[2],c[3]) for c in cards) + '};\n'
header += 'CardDescriptionText* gCardKindDescriptions[98] = {' + ','.join(
 'NULL' if n else '&testDescription' for _,n in entries) + '};\n'
main = r"""
int main(void) {
    DeckMenuWork sora; RikuDeckMenuWork riku; DeckExchangeWork exchange;
    int id, rejected = 0, valid = 0;
    assert(CARD_KIND_MARLUXIA_2 == 95 && CARD_KIND_ANSEM == 97);
    for (id = 0; id < 950; ++id) {
        int kind = gCardDefs[id].kind, missing = kind >= 98 || !gCardKindDescriptions[kind];
        loads = 0; sora.textSlotCount5 = riku.textSlotCount5 = exchange.textSlotCount5 = 123;
        LoadCardDescriptionText(&sora,id); LoadRikuCardDescriptionText(&riku,id);
        LoadDeckExchangeCardDescriptionText(&exchange,id);
        assert(loads == (missing ? 0 : 3));
        assert(sora.textSlotCount5 == (missing ? 0 : 7));
        assert(riku.textSlotCount5 == sora.textSlotCount5 && exchange.textSlotCount5 == sora.textSlotCount5);
        if (kind == CARD_KIND_MARLUXIA_2) { assert(!CollectableCard(id)); ++rejected; }
        if (CollectableCard(id)) { assert(!missing); ++valid; }
    }
    assert(rejected == 11 && valid > 0);
    printf("PASS 950 cards x 3 preview paths; excluded all 11 boss-only Marluxia 2 IDs; %d eligible cards\n", valid);
}
"""
includes = [ROOT/'include',ROOT/'port/include'] + [p for p in (ROOT/'include').iterdir() if p.is_dir() and p.name!='gba']
for region in ('eu','us','jp'):
    source = OUT / (region+'.c'); source.write_text(header+body+main,encoding='utf-8')
    exe = OUT / (region+'.exe')
    subprocess.run([str(CC),'-O1','-mno-ms-bitfields','-std=gnu11','-DPLATFORM_PC','-DVERSION_'+region.upper(),
        *['-iquote'+str(p) for p in includes],str(source),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('PASS EU/US/JP, actual metadata and original description availability; no game save modified')
