# ReKHCoM v1.9.3 Nightly 2 — 2026-10-18

This is a public test build, not a stable release. It includes the accepted
Guard Armor patio and the accepted cutscene profiles for 057 (with completed
grass), 061 and 074. The profiles sample the installed ROM's original tiles
and the game's current palette. No ROM assets are bundled.

These presentation changes have been checked at 16:9 and 32:9, including
pixel comparisons against the same executable with the continuation disabled.
They are not a full playthrough or verification of every camera/zoom setting.
The other seven boss scenery improvements are still pending.

It also retains the optional save-slot selector from Nightly 1.

To try it, open **Options → Experimental** and turn on **Choose save slot at
save points**. At a save point, use Up or Down to choose the Sora or Riku save
slot before confirming. The setting is Off by default; with it Off, the game
keeps its original save-slot behavior.

Please test with a copy of your save files first. Check that you can select
both slots, that the save summary follows the selected slot, and that writing
one slot leaves the other intact. This flow has passed compile and menu-toggle
checks, but has not yet received a full manual gameplay test.
