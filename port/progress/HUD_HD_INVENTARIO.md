# HUD HD: preliminary version acceptance checklist

All replacements are optional under Experimental → HD HUD (game/launcher),
off by default. Native anchors, element sizes and internal positions are the
reference. AI placeholders are allowed; opaque backgrounds and unreadable
lettering are not. Battle HUD work covers Sora and Reverse/Rebirth. Claude's
menu and side-margin work remains separate.

Each graphics family contributes one eighth of the preliminary graphics bar:

- [x] Enemy HP pilot: PNG frame/letters, native bounds, palette gradient fill.
- [ ] Player HP frame(s), including extra bars.
  Native-driven smooth live fill is implemented; complete generated frame,
  secondary layers and damage/healing state coverage remain pending.
- [ ] Player portraits and portrait surround (Sora/Riku, expression states).
- [ ] Card frames and card face/icon artwork.
- [ ] Reload gauge/counter and related prompts.
- [ ] Stocked cards/sleight indicators and totals.
- [ ] Friend card indicators and positioning variants.
- [ ] Battle indicators/tutorial pointers and HUD notices.

Lettering bar: eight families, each with six language coverage slots
(EN/FR/DE/IT/ES/JA). Enemy vitality labels are the first family: HP for
EN/FR/JA, VT for ES, LP for DE, PV for IT. Other families are reload labels,
deck-counter labels, stock/sleight headings, friend labels, boss labels,
tutorial prompts and battle notices. Count a slot only when its preliminary
HD replacement is integrated and readable in native bounds. Existing dialogue
HD text does not count toward these HUD slots. 6/48 are currently integrated.
Variable values use the vector outlines in KHMenu.otf, per the user's later
instruction: match HD text quality, preserve original shapes where feasible.
The CFF outlines contain all ten distinct digits. Numeric bar has six families:

- [x] Sora/Riku card values: native ink bounds, affine motion, HUD occlusion.
- [ ] Remaining-card counters.
- [ ] Reload counters.
- [ ] Stock/sleight totals.
- [ ] Boss card values.
- [ ] Symbols and numeric prompt variants.

No static value is baked into an image. Static HUD lettering still uses image
redraws; the vector menu face may be used where no suitable original exists.

Integration bar: six gates: optional menus/config, transparent PNG loader,
native geometry, image-letter/digit replacement, complete asset packaging,
clean disable/missing-pack fallback across the inventory. First three gates
are available for the enemy pilot; full-coverage gates remain pending.

Validation bar: four gates: EU, US, JP smoke coverage across the inventory,
plus frame-time/memory and geometry comparisons. Pilot-only tests remain
evidence of the pilot; they do not complete whole-inventory validation.

Next: export original HUD pieces with palettes/transparency to create precise
editing references; generate lettering and player HP frame prototypes first.

Developer capture: `KHCOM_HUD_EXPORT=1` saves deduplicated original OBJ images
and an index under the isolated test's `hud-source/`. These ROM-local references
stay under build/ and must never be packaged or committed. A basic EU battle
yielded 25 unique HUD pieces; other states/campaigns remain to be captured.
