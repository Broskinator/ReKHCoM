# Changelog

## v0.5 — 2026-10-13

Windows x86 prerelease.

- Movie subtitles in HD text, in Comic Hearts (a fan-made recreation of the
  game's movie font), centred where the cartridge puts them.
- Movies: the side bars show a blurred reflection of the picture instead of
  its stretched edge column.
- The pause menu list is back (v0.4 could show the pause menu without it);
  the room's save point and map icon stay up through the save menu instead of
  vanishing and popping back.
- State and slot notices are drawn on a layer of their own above everything
  else, at the window's scale.
- HD text reaches menu text (the save menu, lists, names), on the menu layer.
- Start + Select opens the options anywhere; Select alone only on the title
  screen (Start alone no longer opens them).
- Verified without a crash: all 194 story events (European, US and Japanese
  builds) and all 209 battles of the debug battle menu as Sora (European and
  US) and as Riku.
- Return stocked cards (optional, on by default): holding V, or clicking or
  pushing down the right stick, for half a second puts Sora's stocked cards
  back in the hand, in their places in the deck (idea from ggdrn/KH_COM_VITA).
  Toggle in the in-game options and the launcher; bindable in the launcher.
- Launcher install tests (`port/tools/test_launcher.py`): the three dumps,
  reinstalling, wrong size, wrong SHA-1, a non-KH file, a missing file and an
  unwritable roms folder; 10/10 pass.
- Audio compared with BizHawk (`port/tools/audio_compare.py`): boot and title
  music match note for note (sample correlation 0.90, 4 ppm drift).

## v0.4 — 2026-10-13

Windows x86 prerelease.

- Launcher: game language for the European version, a save card (date of the
  save, open the folder, back it up with a date stamp), launcher language
  (system, Spanish, English), version shown in the header.
- Japanese version: HD text in Japanese (Yu Gothic UI on the game's grid) and
  the options overlay in Japanese, drawn with the cartridge's own font.
- No more pops around menus: field objects and the room's map icon stay up
  through the pause and save menus instead of vanishing and coming back; a
  sub-menu no longer shrinks for a frame when it opens.
- Audio: band-limited Game Boy square channels (less harsh high notes); each
  sound FIFO follows its own timer and DMA.
- Right stick: left / right change cards, up stocks them.
- Verified: all 194 story events (European and US builds) and all 209 battles
  of the debug battle menu as Sora (European and US) and as Riku run without a
  crash.

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
