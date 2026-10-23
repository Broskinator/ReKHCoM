"""Export original Sora field idle/walk frames and their exact timings from a local EU ROM.
No generated or original artwork is stored in Git. Requires Pillow and generated ROM symbols.
Usage: python port/tools/export_sora_field.py build/sora-reference
"""
import json
from pathlib import Path
import struct
import sys
from hdcards_gba import Rom
root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
rom = Rom(root)
records = []
for direction in ("bb","ff","fl","ll","bl"):
    for action, code in (("idle","00"),("walk","01")):
        stem = "gSor1"+direction+code
        ptr = struct.unpack("<I",rom.read(stem+"Anims",4))[0]-0x08000000
        origin_x,origin_y,count = struct.unpack_from("<HHH",rom.rom,ptr)
        steps = [struct.unpack_from("<HH",rom.rom,ptr+6+i*4) for i in range(count)]
        for index in sorted({step[0] for step in steps}):
            frame = stem+"Frame"+str(index)
            image,(x,y) = rom.render(frame,stem+"Tiles","gSoraPalette")
            image.save(out/(frame+".png"))
            records.append(dict(name=frame,frame=frame,tiles=stem+"Tiles",direction=direction,action=action,
                                box=[x,y,image.width,image.height],anim_origin=[origin_x,origin_y],
                                steps=[list(step) for step in steps]))
(out/"inventory.json").write_text(json.dumps(records,indent=2)+"\n",encoding="utf-8")
print(len(records),"frames exported to",out)
