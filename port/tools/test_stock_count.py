"""Execute the real sleight predicates for EU/US/JP with 0-3 stocked cards."""
from pathlib import Path
import subprocess
R=Path(__file__).resolve().parents[2]
source=(R/'src/card/mode_riku_tutorial.c').read_text()
a=source.index('s32 IsKindPairThenNonSummonOfCategory(');b=source.index('s32 IsKindThenTwoAttackCards(',a)
out=R/'build/stock-count-test';out.mkdir(exist_ok=True)
fixture=out/'stock.c'
fixture.write_text('#include "card.h"\n#include <assert.h>\n#include <stdio.h>\n'+source[a:b]+r"""
int main(void) {
    CardDisplayWork* empty[3]={0};
    CardDef defs[3]={{0}};
    CardDisplayWork work[3]={{0}};
    CardDisplayWork* cards[3]={&work[0],&work[1],&work[2]};
    for(int n=0;n<3;n++) {
        assert(!IsKindPairThenNonSummonOfCategory(empty,n,37,42,CARD_CATEGORY_MAGIC));
        assert(!IsKindPairThenSummon(empty,n,37,42));
    }
    for(int n=0;n<3;n++) work[n].cardDef=&defs[n];
    defs[0].kind=37;defs[1].kind=42;defs[2].category=CARD_CATEGORY_MAGIC;
    assert(IsKindPairThenNonSummonOfCategory(cards,3,37,42,CARD_CATEGORY_MAGIC));
    assert(!IsKindPairThenSummon(cards,3,37,42));
    defs[2].flags=CARD_DEF_FLAG_SUMMON;
    assert(!IsKindPairThenNonSummonOfCategory(cards,3,37,42,CARD_CATEGORY_MAGIC));
    assert(IsKindPairThenSummon(cards,3,37,42));
    defs[0].kind=100;
    assert(!IsKindPairThenSummon(cards,3,37,42));
    puts("Stock predicates: incomplete stocks rejected; valid summon/non-summon combos preserved");
    return 0;
}
""")
cc=next((R.parent/'toolchain').glob('llvm-mingw-*/bin/i686-w64-mingw32-clang.exe'))
includes=[R/'include']+[p for p in (R/'include').iterdir() if p.is_dir() and p.name!='gba']
for region in ['EU','US','JP']:
 cmd=[str(cc),'-O2','-std=gnu11','-mno-ms-bitfields','-funsigned-char','-DPLATFORM_PC','-DVERSION_'+region,'-I'+str(R/'port/include')]
 for path in includes:cmd+=['-iquote',str(path)]
 exe=out/(region+'.exe');subprocess.run(cmd+[str(fixture),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True);print(region,'passed',flush=True)
