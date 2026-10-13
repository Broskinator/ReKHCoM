# Changelog

## v0.3 — 2026-10-13

Windows x86 prerelease with a launcher and all three cartridge versions.

- ReKHCoM launcher: installs a European, US or Japanese dump (checked by SHA-1),
  edits the settings, enhancements and controls, and starts the matching game.
- Builds for the European (`B8CP`), US (`B8CE`) and Japanese (`B8CJ`) cartridges,
  each with its own save file.
- Remappable controls: two keys and a gamepad button per action; the right
  stick changes cards and stocks them.
- Menus in 16:9: the pause menu, save, world select and level up fill the
  window's height, anchored to both edges, with no pop when they open, close
  or enter a sub-option; full-screen sub-menus keep the GBA framing at full
  height; each screen's side bars match it (title, copyright, language select).
- Battles: arenas repeat sideways in the wide view; the card hand is lifted
  8 px (option) so whole cards show.
- HD text in KHMenu, a fan-made recreation of the game's font, fitted to the
  game's line widths.
- Skip the opening (A, then A again).
- Fixes: crash when reloading cards after the tutorial, stray pointers and
  division by zero behave as on the GBA, a movie skipped early no longer
  corrupts the heap, low HP after the intro, centred message boxes without a
  portrait, letters merging in HD text.
- Verified: all 194 story events (European and US builds) and all 209 battles
  of the debug battle menu run without a crash.

The level-up screen bands, the save crystal exit and the door menu in 16:9 are
still being worked on. Full playthroughs remain unverified. A user-supplied ROM
is required and is not included.

## v0.2 — 2026-10-12

Windows x86 prerelease.

- Play the ROM's opening, event and ending movies with native C codecs.
- Preserve original pixel arithmetic, motion compensation and ADPCM audio.
- Service display, subtitles, audio and input while waiting for movie frames.
- Verify all 3,605 movie frames and 3,604 audio blocks against the ARM decoder.
- Run one game update per VBlank (59.7275 Hz), measured against hardware-accurate
  emulation; the speculative game speed option is removed.
- Show display and logic frame rates with the FPS readout.
- Draw the battle HUD on its own layer, scaled to the resolution ("HUD size"
  option); tutorial pointers follow the HUD element they point at.
- HD text option: dialogue drawn with an outline font at the window's resolution.
- One-button dodge roll and one-button stock (L+R), each optional.
- Executable back to about 14 MB.

Movie event/ending transitions during gameplay and full playthroughs remain
unverified. A user-supplied European `B8CP` ROM is required and is not included.

## v0.1 — 2026-10-12

Initial Windows x86 prerelease of the native PC port.

- Compile all 269 decompiled game C units natively using clang and SDL3.
- Boot through language selection and title into the opening gameplay.
- Render wider field maps and battle arenas, with anchored HUD and messages.
- Provide graphics, display and sound options through an in-game overlay.
- Support keyboard and gamepad input, fullscreen and original-speed timing.
- Write cartridge SRAM saves to disk and support build-specific save states.
- Package the Windows executable, SDL3, documentation and license notices.

Local verification covers startup, title, new game and existing early-game
checks. Full Sora and Reverse/Rebirth playthroughs remain unverified. Movies
are skipped; native movie decoding is pending. Rendering and audio remain
experimental. A user-supplied European `B8CP` ROM is required and is not included.
