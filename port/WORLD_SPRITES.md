# Experimental world-sprite artwork

The first local artwork is gSor1fl00Frame0.png: Sora's front-left field idle pose. The game mirrors this same artwork for front-right. This is one supplied pose, not complete character or animation coverage. The generated walk sheets were rejected for repeated stride poses and unwanted background; no rejected walking artwork is installed. Walking and other poses retain original sprites.

## Runtime

hd_world_sprites=1 enables the pilot, independently of HD cards/portraits and pixel filters. The launcher exposes HD world sprites (pilot). Missing files retain native sprites. World assets and fitting metadata are loaded before the native frame is suppressed, so invalid files retain a visible native character.

port/data/hd_world_sprites.tsv registers 45 original idle/walk frames across five source directions. The other three facing directions use the original game's mirroring. Frame pointers and original OAM parts select the artwork; frame timers, player position, collision and input logic are unchanged. Registration metadata does not imply artwork exists for every frame.

The existing world-layer path handles transforms and native foreground occlusion. Optional PNG.fit files contain eight integers: source pixel x,y,width,height and destination logical x,y,width,height inside the original frame. Loading fits the art into a transparent 16x-resolution frame once, so generated margins cannot move the foot anchor. Invalid bounds reject the replacement. Existing assets without fitting metadata use their previous path.

## Extraction

With a local EU ROM and generated symbols, run python port/tools/export_sora_field.py build/sora-reference (requires Pillow). It exports the original images locally and writes their layout, animation origin and frame durations. The inventory is recorded in port/data/sora_field_inventory.json. Each walking cycle uses eight original frames held for seven ticks each; idle uses one frame. Neither the source images nor the generated artwork are committed to Git.

## Verified and limits

Windows EU game and launcher build; generated-table check; Direct3D11 and software field-room runs; missing/off/invalid-fitting fallback; mirrored pose capture; unchanged logged actor position for the same inputs with HD enabled, disabled, missing or invalid. Mirrored capture was inspected before the room's later story transition. The original foreground boundary occludes the mirrored character. The previous four card-overlap regressions also pass.

No full-playthrough, exhaustive occlusion/window/blending check or finished HD walk cycle is claimed. The art is an AI redraw and needs further consistency review. The prototype will switch between the HD idle and original movement artwork. Finish and review the eight matched walk poses before describing this as an HD idle-and-walk animation pack.
