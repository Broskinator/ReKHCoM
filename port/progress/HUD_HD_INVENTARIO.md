# HUD HD: preliminary version acceptance checklist

All replacements are optional under Experimental → HD HUD (game/launcher),
off by default. Native anchors, element sizes and internal positions are the
reference. AI placeholders are allowed; opaque backgrounds and unreadable
lettering are not. Battle HUD work covers Sora and Reverse/Rebirth. Claude's
menu and side-margin work remains separate.

Each graphics family contributes one eighth of the preliminary graphics bar:

- [x] Enemy HP frame: PNG redraw, native bounds, native fill and lettering.
- [ ] Player HP frame(s), including extra bars.
- [ ] Player portraits and portrait surround (Sora/Riku, expression states).
- [ ] Card frames and card face/icon artwork.
- [ ] Reload gauge/counter and related prompts.
- [ ] Stocked cards/sleight indicators and totals.
- [ ] Friend card indicators and positioning variants.
- [ ] Battle indicators/tutorial pointers and HUD notices.

Lettering bar: six regional language sets (EN/FR/DE/IT/ES/JA), counted only
when image-generated HUD lettering is integrated and readable in its original
boxes. Existing outline HD text is not counted as the image-generated method.
Variable values require reusable digit glyphs and symbol states, not baked
numbers in static pictures.

Integration bar: six gates: optional menus/config, transparent PNG loader,
native geometry, image-letter/digit replacement, complete asset packaging,
clean disable/missing-pack fallback across the inventory. First three gates
are available for the enemy pilot; full-coverage gates remain pending.

Validation bar: four gates: EU, US, JP smoke coverage across the inventory,
plus frame-time/memory and geometry comparisons. Pilot-only tests remain
evidence of the pilot; they do not complete whole-inventory validation.

Next: export original HUD pieces with palettes/transparency to create precise
editing references; generate lettering and player HP frame prototypes first.
