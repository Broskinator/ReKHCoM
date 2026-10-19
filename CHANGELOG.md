# Changelog

## Unreleased

- Achievements: direct catalog with cached badges (grey when locked, colour when earned);
  account login remains in the launcher. Official online awards/Hardcore remain disabled.
- Options → Achievements: read-only local progress and the launcher's last official
  snapshot, with scrolling and locked/unlocked filters; keyboard and gamepad support.
- Local achievements accept Windows/portable progress files; Japanese voices and
  presentation settings no longer invalidate local challenges.
- Web launcher: matching graphics/sound/enhancement/control tabs, optional save-copy
  editor with card limits and backups, and a direct list of the 16 local challenges.
- Dialogue portraits no longer show early or uncovered in wide views (Codex, #59).
- Android: the GBA memory is reserved region by region when the whole range is not free (#20).
- Save crystals, gimmicks and the room's map icon follow the game's own lifetime through
  menus again (no more port-made copies); fixes the crystal that vanished (#63).
- Options: the debug menu's click sounds as the light moves; the music pauses and comes back on the same note.
- Options → Cheats → Debug mode: "[ReKHCoM] GBA map logic" brings back the cartridge's
  240x160 bounds for field Heartless, with an in-menu note explaining the change.

## v1.9.5 — 2026-10-18

Everything from the v1.9.3 test builds, now stable (v1.9.4 was prepared but never
published; its changes are here).

- Options → Cheats (off by default, remembered): one-hit enemies, infinite HP, level 99
  with every bonus, map cards at 9 and a full battle card collection.
- Options → Boss rematches: refight Sora's beaten bosses and the two battle tutorials.
- Optional save-slot selector at save points (Options → Experimental); the save
  crystal refreshes the summary per slot and remembers the last slot used.
- Moogle points notice (Options → Experimental, off by default).
- Options menu: the list now uses the whole height of the window (it stopped three
  rows early).
- Launcher: Save editor tab (level, HP, CP/AP/DP, moogle points, cards; backed up
  before writing), controller drawing with the Xbox-style buttons lit for the action
  being remapped; unknown settings are kept.
- Controls screen in the game: toon controller art highlights the binding being remapped.
- Local achievements for EU/US/JP with centred animated notices (Windows);
  official RetroAchievements is not active yet.
- Wide view: Guard Armor patio and scenes 061 (ice) and 074 (Monstro) extended with the
  game's own tiles; beach 057 included (still being refined); Riku's rooms close the cut
  top wall with the room's own scalloped border (KHCOM_TOPCLOSE=0 turns it off).
- Fixed #16 (black screen at door card select), #17 and #18 (world map bands).
- Opening and closing Options no longer jumps the scene scale.
- Android/Web: option to hide the touch controls; Android and Web alphas updated.
- Linux, Steam Deck and Anbernic alpha: the bundled libraries are found by the names SDL
  asks for (issue #19); HD text looks for a Japanese font in more places.
- PS5 alpha integrated (build needs a native Linux host; not rebuilt in this release).
- You need your own ROM; none is included.

## v1.9.3-nightly.20261018.5 — 2026-10-18 (public test build)

- Riku's rooms: the cut wall at the top of the wide view now closes with the room's
  own scalloped edge turned upside down, over plain wall (no repeated ornaments or
  half window frames). On by default; KHCOM_TOPCLOSE=0 turns it off.
- Local achievements for EU/US/JP: a white Achievements tab and animated notices
  centred at the top (Windows). Official RetroAchievements is not active yet.
- Controls screen: a toon controller drawing (selected art, transparent) lights the
  button bound to the action being remapped; Xbox button names.
- Opening and closing Options no longer jumps the scene scale; the fade covers every
  layer and the selector keeps the original debug light.
- Linux: HD text finds a Japanese font in more places (KHCOM_CJK_FONT, Noto/Takao/IPA,
  fontconfig).
- Windows, Linux, Steam Deck, Anbernic alpha, Android alpha and Web alpha updated.
  The PS5 alpha package is not rebuilt (needs a native Linux host).

## v1.9.3-nightly.20261018.4 — 2026-10-18 (public test build)

- Options → Cheats: enemies fall in one hit, infinite HP (both off by default and
  remembered), level 99 with every level-up bonus (HP, CP and sleights; Riku AP/DP),
  every map card at 9 (the most the game holds) and a full battle card collection
  (999, up to 99 of a card). The actions work while walking a room.
- Options → Boss rematches: refight Sora's bosses already beaten and the two battle
  tutorials. You come back exactly where you were; nothing from the fight is kept.
- Moogle points notice: picking up moogle points shows how many and the total
  (on by default; optional in the game, the launcher and kh_com.ini).
- Launcher: new Save editor tab (off until you accept a warning): level, HP, CP or
  AP/DP, moogle points, map cards and battle cards with their own pictures from the
  cartridge. Every change is listed first and the save is backed up before writing.
- Launcher Controls: an Xbox 360-style controller drawing lights the button bound to
  the selected action; the gamepad column shows Xbox symbols.
- Fixed #16: the door card selection was a black screen in every view except Auto.
- The launcher keeps settings it does not know, so newer game options are not lost.
- Windows, Linux, Steam Deck, Anbernic alpha, Android alpha and Web alpha updated.
  The PS5 alpha package is not rebuilt in this nightly (Nightly 3 package still applies).

## v1.9.3-nightly.20261018.3 — 2026-10-18 (public test build)

- Android/Web: Options → Experimental → Hide touch controls, remembered across
  restarts. The options button remains available; opening Options restores touch
  navigation so the panels can be enabled again, including on handhelds.
- Integrated Claude's save-crystal fixes: changing slots refreshes the summary,
  empty slots clear it, writes target the selected slot, and the crystal remembers
  the last saved slot. The launcher preserves the Experimental preference.
- Updated Windows, Linux, Steam Deck, Anbernic alpha, Android alpha and Web alpha
  packages, and included the integrated PS5 port as alpha (PPSA98511.zip).
- Linux/Web builds now link the portable voice support required by the core.
- Includes the previous nightly's scene and Guard Armor improvements. Other
  reported savestate/art issues remain tracked; this is a prerelease, not stable.

## v1.9.3-nightly.20261018.2 — 2026-10-18 (public test build)

- Guard Armor: accepted patio continuation using original tiles and palette,
  extending the facades and paving without wrapping another complete patio.
  Native 240x160 pixels and Original view stay unchanged.
- Integrated the accepted per-layer cutscene tile profiles for 057 (beach,
  including the completed grass), 061 and 074 (including the completed lower
  ornament). These now run in the game with its live palette and effects.
- Includes the previous nightly's optional save-slot selector, Off by default
  under Options → Experimental. Manual save-point validation is still pending.

## v1.9.3-nightly.20261018 — 2026-10-18 (public test build)

- Experimental save-slot selection at save points. It is Off by default and
  must be enabled under Options → Experimental. This flow has passed build and
  menu-toggle checks but still needs a manual gameplay test; see
  `NIGHTLY-TESTING.md` in the package.

## v1.9.2 — 2026-10-18

- Battle HUD at every aspect ratio (issues #7, #8, #9): Sora's stocked cards
  now sit under his HP gauge instead of floating mid-screen; the opponent's
  deck (Leon's tutorial, Axel, Larxene, Riku, Vexen, Marluxia, Ansem, Lexaeus)
  docks to the bottom-right corner and its stocked cards under its HP.
  Checked at 3:2, 4:3, 5:4, 16:10 (Steam Deck), 16:9, 21:9 and 32:9.
- Cutscenes: the scene where Sora meets Leon continues its floor and walls past
  the map edge in the wide view (original tiles only) instead of a flat band.
- Two original-game bugs that behaved differently on PC: Guard Armor's defeat
  read an uninitialised flag, and World Inspect could write past its list.

## v1.9.1 — 2026-10-17

- Field Heartless no longer vanish in plain sight near the edges of the wide
  view and respawn elsewhere: the original game removes them when they leave the
  240x160 screen, and that limit now follows the visible area on PC.
- Title screen: the illustration fades into white at the edges of wide screens
  using the original art only (text, logo and Riku's title are unchanged).
- Android alpha: launcher icon and HD text option. Web alpha: site icons.

## v1.9 — 2026-10-17

- Riku: single-circle opening, native tile borders in eligible cutscenes,
  persistent optional unlock and access-gated Riku save slots without deleting saves.
- Return stocked cards is available under Experimental, disabled by default.
- Updated Android alpha 3 and browser alpha 3; native Linux builds remain pending.

- Reduce renderer allocation at ordinary zooms and speed up opaque world scanlines
  and straight affine backgrounds. Gameplay speed and original effects are preserved.
- Add 4:3 and GBA 3:2 aspect options alongside widescreen, with zoom and HD text.
- Save imports preserve completion flags when changing region. A damaged first
  header no longer shifts the entire save; a valid backup header can be used.
- Reject ambiguous or invalid save containers. Backups must succeed before an
  existing save is replaced, and failed writes leave the original intact.
- Show the five save banks in the launcher and keep imported saves in the selected bank.
- Clear keyboard, dodge and stock input when focus is lost.


## v1.8.3 — 2026-10-16

- Map Cards: the pointing glove no longer stretches into a white stripe when
  opening or closing the menu, including on ultrawide screens.
- Deck View uses the same protection during transitions. Status keeps its
  corrected animations, including NEW notices.
- Includes the recent audio, cinematic border, room transition and zoom fixes;
  x4 remains the default. Normal saves remain compatible.

## v1.8.2 — 2026-10-16

- Language selection: keep side textures free of repeated emblems throughout
  the fade, and match the blank transition frame's background.
- Title to Load: both margins follow the same exact background colour during
  the fade; dark artwork no longer wins a quantized colour bucket on one side.
- Room changes: blank the display before the next mode reuses graphics memory,
  eliminating the old room's one-frame white silhouettes. Replaces the
  unverified layer-retention workaround.
- Deck View: retain the partial HD-number clipping improvement. Overlapping
  text reported in the live submenu remains a known issue for a future fix.

## v1.8.1 — 2026-10-16

- Status entry/exit draws its margins from background layers. NEW badges and
  other foreground pixels no longer stretch into horizontal stripes.
- Remove intermediate zoom steps from the launcher and in-game selector.
  Keep the numbered scales, x4 default and x6/x3 reference annotations.
- Migrate an old intermediate selection to the next whole pixel scale.
- Audio: no more 60 Hz buzz. The music engine changes each note's volume once
  a frame (envelopes, fades); the mixer now ramps it across the frame instead
  of stepping at every frame boundary.

## v1.8 — 2026-10-16

- Simplify zoom options: remove the integer-only filter and the separate
  scaling row from the launcher and in-game menu. Keep all zoom steps.
- Mark x6 as the integer x1 reference with a warning, and x3 as integer x2.
  Default to x4 at every supported output size; preserve explicit selections.
- EXP gain, next-level and level-up notices always use their original
  pixel-art numbers, even with HD text enabled. Keep HD dialogue, subtitles
  and menus; preserve notice positions and experience/level calculations.
- Extend uniform, opaque map padding beyond cinematic map boundaries. Never
  stretch patterned scenery or change the original GBA image.
- Sprite-only cinematic messages fill their margins from the current rendered
  backdrop, including fades, instead of a previous scene's side treatment.

## v1.7.1 — 2026-10-16

- Audio: interpolation at the end of a PCM sample uses its real loop or edge,
  never the bytes following it; short loops no longer pick up unrelated data.
- Audio: prime the playback buffer at startup and after loading a state to
  protect against initial underruns; reset the queue's drift estimate too.
- Faster PSG square-wave synthesis, with mathematically identical output.
- Faster software PPU: decode wide-world backgrounds by tile spans and share
  visible-layer decisions between composition and HD-text occlusion.
- Reduce idle CPU use in the frame timer; retain independent presentation
  buffers for correct menu fades and ultrawide gameplay.
- Restore v1.4 pixel-scale numbering and its classic preset (x4 at 1080p).
  Integer-only selection now filters real whole pixel scales. Turning it off
  adds explicitly marked intermediate steps, without decimal labels.
- Keep v1.7's aspect ratio and menu transition fixes.
- Normal save files remain compatible. Quick save states are build-specific;
  retain the matching executable for states from earlier releases.

## v1.7 — 2026-10-16

- Zoom x1 to x6: how much world you see, as a multiple of the GBA's view
  (x1 Original GBA, x2 Enhanced, the default). The options show which levels
  are an exact whole-number scale on your screen and which are demanding.
- New option, Integer scale (on by default): sharp whole-number pixels; off
  gives the exact in-between zooms.
- Aspect ratio: Force 21:9 and Force 32:9, an ultrawide picture in any window.
- Fullscreen resolution: browse with left/right, press A to apply.
- Menus: their side extensions no longer blink while a menu slides in or out
  (the margins follow the sprites of the same frame).
- The options menu is drawn above the battle HUD and the menu layers.

## v1.6.7 — 2026-10-15

- Zoom is the whole-number pixel scale, every step from x1 (the most world) to
  Original (the GBA's 160 lines, x6 at 1080p). Enhanced, x4 at 1080p, is the
  default. Steps below full speed (x1, x2 at 1080p) are marked demanding, with
  a warning in the options. The picture buffers moved to the heap so x1 fits.
- The options menu stays readable at any zoom.
- Battles: the enemy's played card is drawn at the HUD's size, on the bottom
  row at the height of Sora's cards, above its enemy.

## v1.6.6 — 2026-10-15

- Zoom: the default step is called Enhanced (x3 at 1080p). The farther steps
  (x2 at 1080p) are available again, marked as demanding: they show more of
  the world but need a fast CPU for 60 fps.

## v1.6.5 — 2026-10-15

- Full speed at every zoom: the far steps that made the software renderer
  draw more than it can at 60 fps (x2 at 1080p, 960x540) are no longer
  offered; on very large or ultrawide screens Normal picks a scale the
  renderer keeps up with.

## v1.6.4 — 2026-10-15

- Older settings go back to Aspect ratio: Fill window once, so the picture
  follows the window when it is resized (16:9 and 21:9 remain as options for
  very wide monitors).

## v1.6.3 — 2026-10-15

- Room synthesis is a full-screen menu again (its title band and bars reach
  the screen's edges); only the door's spotlight before it fills the height.

## v1.6.2 — 2026-10-15

- Menus and every GBA-size scene fill the screen again at any zoom (no
  smaller menus with blurred bands).
- Zoom: Normal, the default, is x3 on a 1080p screen.
- Hitting a door: the spotlight scene fills the screen's height.
- A battle's opening (the heart bursting) covers the whole screen.
- Menu headers stay on top while they slide; World Cards and the Journal
  slide their headers through the margins too.

## v1.6.1 — 2026-10-15

- Title, language and other GBA-size screens fill the height again at every
  zoom; only the menus over the field follow the zoom.
- Zoom shows its pixel scale again (Normal x4 and so on), in the game and the
  launcher.

## v1.6 — 2026-10-15

- Menus follow the zoom: at Normal they fill the screen's height as before;
  the far steps make them smaller in the same proportion as the world, with a
  soft blurred backdrop above and below.
- Menu headers in the side margin travel with their slide-in and slide-out
  animation, as if the screen had always been that wide.
- Options: the debug-menu picture of Sora fills the screen's height.

## v1.5 — 2026-10-15

- Fullscreen uses the monitor's own resolution by default, so ultrawide
  monitors (5120x1440 and others) fill the screen; the launcher lists the
  monitor's resolutions.
- Zoom: Normal shows the world at x4 on a 1080p screen; no two steps give the
  same scale; the closest step is Original (GBA), the cartridge's view at full
  height.
- Battles at far zooms: the stage carries on below instead of a black band.
- Field minimap stays in its corner at every zoom, a little in from the edge.
- Room synthesis: the cursor, hand and card name line up with the cards
  again, also after the event-door tutorial; the door's shade covers the whole
  screen.
- The Re:Chain of Memories version belongs to each save, chosen at New Game:
  no switch in the options; loading a save brings back its settings.
- Options trimmed: Filter, raised battle cards and save banks are internal
  now; Smooth audio is called Audio filter; the FPS counter is smaller.
- HD-2D look hidden until it is finished, like the HD HUD.

## v1.4 — 2026-10-15

- HD text: field dialogue shows its HD letters again. Since v1.0 the message
  windows marked their own area as covered, so with HD text on the dialogue
  boxes came up empty.
- Side margins: no more black column at the right edge of the picture at
  window sizes such as 1920x997 (a window pixel on the picture's exact edge
  was drawn by neither the picture nor the margin).
- Menu headers in the margin appear together with the header, not before it,
  while a menu slides in.
- Deck and other menus: the header's line reaches the margins together with
  the menu's slide-in.
- World map: the floor badge (P 1) no longer has the coloured bands drawn
  over its continuation while it slides in.
- Save files (New game and Load): side margins drawn from their own scene,
  with the frame's bars and the title band carried on (the 18th screen).
- New game: the first question no longer shows "B Back" (there is nothing
  before it).
- Zoom (Auto view): every step now shows more of the world than the one
  before, and a new step, Farthest, makes everything smaller (as on a 32:9
  monitor) on a 16:9 screen. The option shows the pixel scale it gives.
- HD HUD: pending until it is finished; forced off for players.

## v1.3 — 2026-10-15

- The opening movie's "press again to skip" prompt keeps its normal size and
  place when movies are shown cropped and enlarged; it no longer grows with
  the video and runs off the screen.

## v1.2 — 2026-10-15

- New game always asks first which version to play, the original or
  Re:Chain of Memories (difficulty, HD text and one-button controls); before,
  the question only appeared with the experimental Re:CoM switch on. The
  following screen starts with that version's presets.
- README: no development progress chart; development notes moved out of the
  repository root.

## v1.1 — 2026-10-15

- Ultrawide and other window shapes: every screen checked at 16:10, 16:9,
  21:9 and 32:9 (1152x720 to 2560x1080). Menu margins are now drawn column by
  column on the picture's own pixel grid, so at non-integer scales such as
  6.75 (2560x1080) their stipple and checkerboards no longer slip by a pixel.
- New video option, Aspect ratio (game and launcher): fill the window, or keep
  the picture at most 16:9 or 21:9 with black bars at the sides on wider
  monitors.
- README: ultrawide screenshots.

## v1.0.1 — 2026-10-14

- Fix: starting a new game in the Original version no longer turns off the
  one-button controls (dodge roll, stock and return stocked cards); they stay
  as set in the options (on by default, as before v0.8). Re:CoM still turns
  them on.
- Test harness: a virtual gamepad (KHCOM_VPAD) and exact window sizes
  (KHCOM_WINDOW) for automated checks.

## v1.0 — 2026-10-14

First public release (Windows x86).

- Side margins, finished: the 17 GBA-size screens (journal, decks, status,
  map and world cards, Moogle Shop, world map, world select, vortex, elite
  bonus, START menu, room synthesis, save, level up, language select,
  copyright and title, movies and battle entry) reviewed one by one. Every
  margin is drawn from the screen's own art on the picture's exact pixel
  grid: bars, boxes, titles and panels carry on to the window's edges, menu
  stipple and world-map checkerboards keep their phase, plain backgrounds
  stay plain (no mirrored shapes) and nothing is stretched.
- Menu titles: the arrow's tip meets the line drawn from it.
- Movies in Fill height mode: cropped to the video, without its letterbox,
  and shown as large as the window allows at the video's own proportions.
- Fill height is the default for GBA-size scenes (older settings switch to
  it once).
- Room synthesis: the card panel closes above the bottom bar.
- Wide menus reach the window's right edge at every size (START menu,
  save, room synthesis at 1280x720 lost their last 3 pixels).
- Launcher: its language is the system's the first time and stays as
  chosen (Español / English); fixed accented text.
- The HD HUD pilot is hidden until it is finished.

## v0.8 — 2026-10-14

Windows x86 prerelease.

- Experimental Re:CoM mode (PS2 options) chosen in a new-game window:
  version (Original / Re:CoM), difficulty, HD text and QoL buttons; each
  save keeps its own QoL settings. With the mode off nothing changes from
  the cartridge.
- Side margins with no stretched pixels: menus continue their own tiles and
  stipple, full-height scenes get margins drawn by the PPU from their own
  layers, live scenes a blurred reflection with their bars drawn on, and the
  journal its green paper. Every margin row and column sits on the
  picture's own pixel grid, so bars, boxes and panels line up exactly.
- Menu headers keep a clean band in the margin in every region.
- Launcher: import a GBA save from any emulator or backup device
  (experimental).
- Experimental HD HUD pipeline (AI-redrawn enemy HP frame, off by default).
- Level-up: the stat panel no longer wraps into the left margin on exit;
  the door menu sits above the bottom bar.
- HD text covers the numbers that are text: deck-menu counts and CP, EXP and
  next level, with the colours, height and baseline of the pixel digits.
- Fix: the packaged game (in bin/) now finds the HD text fonts in assets/.
- Battle card breaks pixelate the arena too: affine backgrounds take the
  mosaic, as on the GBA (only the sprites did before).

## v0.7 — 2026-10-14

Windows x86 prerelease.

- Experimental smooth audio: Catmull-Rom interpolation of the samples in the
  m4a mixer and a 2-pole low-pass at 9 kHz on the output; the treble images
  of the GBA's low sample rates drop by 5-15 dB, the rest is unchanged.
- The launcher is the only executable at the root of the package; the game
  executables moved to bin/ (the launcher still finds them at the root in
  older installs).
- Icon (the Kingdom Hearts heart) for the launcher and the game, and the
  Re:Chain of Memories logo in the launcher's header.
- HD-2D: less bloom and no haze in scenes that are bright all over (the white
  castle); message windows stay untouched.

## v0.6 — 2026-10-13

Windows x86 prerelease.

- Battle entry: the margins of the 16:9 window show a blurred copy of the
  picture, as the movies do, and the animation keeps its proportions.
- World cards: the margins fade out in pixel art without stretching the
  cards or the foreground.
- World map: the animated ornamental bands extend from the original art; a
  transition regression test was added.
- Experimental section in the in-game options and the launcher, with HD text
  and a new HD-2D look: depth of field (the distance and the foreground blur,
  the characters stay sharp), bloom, filmic grading, haze and a vignette, in
  the manner of Octopath Traveler. Off by default; the alternative build
  (`configure_pc.py --hd2d`, build/pc-hd2d) has it on.
- PSG: channel 1's sweep checks the next step for overflow as the hardware
  does (at the start and after each update); noise with a shift of 14 or 15
  stops, as on the hardware.
- `port/tools/test_songs.py` plays every song and sound effect of the ROM and
  checks for sequencer errors, silence and clipping.

## v0.5 — 2026-10-13

Windows x86 prerelease.

- Movie subtitles in HD text, centred where the cartridge puts them.
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
- Save banks: five banks of the game's two slots (bank 1 is the usual
  kh_com.sav, bank n kh_com_bank<n>.sav, per version). Chosen in the in-game
  options (switches at once, writing the current bank first) or on the
  launcher's save card, which follows the selected bank.
- Save states verified deterministic (`port/tools/test_savestate.py`): loaded
  in a new process, a mid-battle state plays on pixel-identical (3/3).
- The options screen takes the look of the cartridge's own debug menu: white,
  grey bars with the red cursor square, and its picture of Sora (decoded from
  the ROM). The skip-intro prompt, the state notices and the title's options
  hint use the same button, and so does the FPS readout. The options screen
  fades in and out through white, as the game's menus do.
- The launcher takes the same look: white, the grey of the bars, black text
  and the red cursor square for headings, toggles, the version list and Play.
- Launcher install tests (`port/tools/test_launcher.py`): the three dumps,
  reinstalling, wrong size, wrong SHA-1, a non-KH file, a missing file and an
  unwritable roms folder, English interface; 11/11 pass.
- Audio compared with BizHawk (`port/tools/audio_compare.py`): boot and title
  music match note for note (sample correlation 0.90, 4 ppm drift); the
  castle movie's streamed audio matches at 0.99. The port plays 3.5 dB quieter.

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
- HD text fitted to the game's line widths.
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
