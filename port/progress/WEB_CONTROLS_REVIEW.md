# Web controls and mobile files — isolated review branch

Branch: `fix/web-mobile-files-controls`. Not merged or published. Menu scaling
belongs to `feature/relative-menu-scale` and must not enter stable releases yet.

Implemented:
- SDL keyboard target is #canvas rather than #window, so toolbar/file controls
  do not send game input. Focus loss, pointer cancellation and fullscreen change
  clear held keys/touches. Native resize clears touches before remapping geometry.
- Focus loss clears pending dodge/unstock actions too.
- ROM and SRAM pickers have no extension filter that might disable unknown GBA
  file types in iOS Files. ROM header/size/SHA1 and save size validation remain.
- Export prepares a local backup, then exposes a persistent Download .sav link
  and, when supported, a fresh-click Share / Save to Files action. Async save sync
  cannot consume the share button's transient activation. Cancelling is harmless;
  unavailable sharing leaves the download link usable. Nothing is uploaded.
- Import targets the configured bank, resets the file input for repeated selection,
  and no longer reports a successful persistent import after a sync failure.
- Browsers without element fullscreen have a viewport expansion mode with a
  visible exit button. Escape leaves that mode. EN/ES copy covers new controls.

Verified:
- test_web_runtime.cjs PASS: existing sizing/storage cases plus focus/cancel,
  fallback viewport, exact 64 KiB synthetic SRAM import/export, bank 3, invalid
  import preservation, share cancellation and share-to-download fallback.
- test_web_controls.py PASS: all eight touch buttons plus diagonals/multi-touch/
  cancellation/focus at 5 aspect ratios; 13 keyboard actions, opposite directions,
  stock/dodge/focus reset, and a real SDL virtual gamepad.
- test_android_touch.py PASS: existing mobile touch and options routing.
- Actual Chrome + Emscripten/SDL input probe (no ROM): all 13 keys observed
  (mask 8191), 13 key-downs, held=0 after release. Typing xazscdv in an external
  input kept presses=13 and held=0: canvas keyboard isolation works.
  Evidence: build/web-controls-browser-proof.jpg. Initial probe-only callMain
  export omission was fixed before these observations.

Pending:
- EU/US/JP web rebuild completed successfully. Inspect new setup/export UI in Chrome.
- Actual game/browser checks: ROM selection through the extension remains blocked
  by its file-URL permission. User can select manually; do not bypass with a ROM
  HTTP endpoint or script injection. Public Alpha 2 was opened, not modified.
- Physical Android Chrome and iPhone/iPad Safari file import/export, touch, rotation,
  fullscreen fallback and Gamepad API. Desktop virtual gamepad is not a device test.
- No deploy or stable promotion until scope and validation are recorded.
