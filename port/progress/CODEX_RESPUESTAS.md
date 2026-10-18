# Respuestas propuestas para Claude

No publicar desde esta rama. Revisar e integrar primero.

## #14 — Undub

An optional Japanese-voice mode is now implemented for the European and US builds. It reads the verified original Japanese B8CJ ROM from `roms/B8CJ.gba` at startup; no patched ROM or audio is distributed. Install the Japanese ROM through the launcher, enable **Japanese voices (restart)**, then relaunch. The original regional text and gameplay remain in place. The option stays disabled when the required Japanese ROM is absent. We are finishing audio-dump verification before calling this fully validated.

## #13 — Remap triggers

The PC port now supports remapping the analog triggers as well as the other gamepad buttons, in both the launcher and the in-game Controls menu. The in-game menu can bind keyboard keys, gamepad buttons, or analog axes and can restore the original bindings. Please try it with your controller and report any device-specific mapping issue.

## #12 — Choose a save slot at save points

Save points now let you choose File 1 or File 2 with Up/Down before confirming. The selected slot is shown in the save panel, and saving there does not switch the active in-game file. Please try selecting both slots and let us know if the prompt could be clearer.

## #6 — Proud wording

The difficulty descriptions now state the actual damage multipliers used by this port. Beginner makes Sora take 50% damage and makes enemies take 125%; Proud makes Sora take 150% damage and makes enemies take 80%. Standard retains the original cartridge damage. These are damage multipliers only: this port does not change enemy AI, HP, card values, or card behavior based on difficulty. The text has been updated in all supported in-game languages.

Posted 2026-10-18: https://github.com/MatyMeatBoy/ReKHCoM/issues/6#issuecomment-6246564794

## #5 — 16:10 view

The Automatic view already adapts the internal canvas to the window's aspect ratio, including 16:10. At 1280×800 it selects 256×160 (a 5× scale). Please try **View → Automatic** and let us know if you were asking for a separate fixed-size preset instead.

Posted 2026-10-18: https://github.com/MatyMeatBoy/ReKHCoM/issues/5#issuecomment-6246564791

## #4 — Zoom levels

The zoom selector offers resolution-aware scales. On a 2880×1800 display it reaches ×11, so you can select a larger scale than ×4 or ×5. We verified 56 resolution/zoom combinations, including that display size. Choose **View → Automatic**, then adjust **Zoom** in Options.

Posted 2026-10-18: https://github.com/MatyMeatBoy/ReKHCoM/issues/4#issuecomment-6246564797

## #3 — Missing larger HUD text

Thanks for the report. Could you share which screen or HUD element was missing the larger text, and whether HD text was enabled? We cannot reproduce the issue from the description alone; if it happens again, a screenshot and the view/zoom settings would help us narrow it down.

## #2 — Remake story features

Thanks for the suggestion. “Story features” could refer to several different changes, and the request does not name a specific scene or mechanic. Which feature from the remake would you most like to see first? That will help us estimate the scope and avoid promising a broad remake before we know what you mean.
