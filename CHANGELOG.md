# Changelog

## Unreleased

- Play the ROM's opening, event and ending movies with native C codecs.
- Preserve original pixel arithmetic, motion compensation and ADPCM audio.
- Service display, subtitles, audio and input while waiting for movie frames.
- Verify all 3,605 movie frames and 3,604 audio blocks against the ARM decoder.
- Verify the full opening and return to gameplay in English and Spanish,
  including widescreen and the alternate gameplay speed setting.

Movie event/ending transitions during gameplay and listening checks on real
audio hardware remain pending. The v0.1 release below is the earlier baseline.

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
