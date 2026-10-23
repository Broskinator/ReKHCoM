# HD artwork pilot

This branch begins asset-level HD replacement. The earlier Scale2x filter is not an HD art remaster; keep filter=0 to evaluate this pilot.

The local pilot contains three AI-redrawn Sora HUD portraits: normal, hurt and low HP. Each is a transparent 1385x1136 RGBA texture replacing a 39x32 logical sprite. The original expression selection and game logic remain in control. These are artistic reconstructions, not recovered original high-resolution drawings or pixel-exact super-resolution. Their style and silhouette need user review before producing a full pack.

## Installation

Place the three locally supplied gBtlHpSoraFaceTiles_0.png, gBtlHpSoraFaceTiles_1.png and gBtlHpSoraFaceTiles_2.png in hd/sprites/ beside the data/settings folder. Enable Graphics > HD portraits (pilot) in the launcher, or set hd_portraits=1 in kh_com.ini. This toggle is independent of the imported Re:CoM cards. Missing portrait files retain the original sprites. Turn the option off to restore original portraits. No HP-ring replacement is supplied.

Generated artwork and ROM-derived references are kept in the local deliverables, not committed to this repository. Prompts and file checksums accompany the local pack.

## Verified

Windows EU game and launcher build. An isolated battle drew the normal portrait at frame 160 and the hurt portrait at frame 240, confirmed in the HD draw log. Both use filter=0 and hd_cards=0. Off/missing-file and software-renderer checks are recorded in the local pilot report. The low-HP PNG has been inspected and validated as RGBA, but its low-HP transition has not been exercised in gameplay. No full playthrough has been verified.

## Full-game work remaining

The three portraits are a prototype, not complete HD coverage. Playable characters need consistent high-resolution artwork for every animation frame and direction, matched to original frame bounds, anchors and palette variants. Enemies need the same treatment. Environments need reconstructed tile/background artwork and a high-resolution replacement path that preserves scrolling, tile reuse, palette effects, occlusion and battle transitions. Cards, UI art, title art and movies need their own reviewed assets. The current port already maps HUD/card replacement PNGs, but it does not provide a complete high-resolution replacement system for all dynamic world graphics.

A faithful remaster requires asset extraction and inventory, a reviewed visual style, replacement rendering for world assets, then systematic coverage and scene testing. Enlarging the final framebuffer cannot complete those steps.
