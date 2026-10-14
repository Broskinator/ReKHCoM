#ifndef GUARD_PORT_UI_H
#define GUARD_PORT_UI_H

#include <stdint.h>

void PortUiInit(void);
/* Draw overlays (options menu, notices) on top of the finished frame. */
void PortUiDraw(uint32_t* frame, int width, int height);
int PortUiDrawNotice(uint32_t* frame, int width, int height);
/* The FPS readout (show_fps), drawn on the notice layer while no notice is up. */
void PortUiSetFps(float fps, float logic);
/* Returns non-zero when the event was consumed by the UI. */
int PortUiHandleEvent(const void* sdlEvent);
/* Non-zero while an overlay owns the input. */
int PortUiActive(void);
void PortUiOpenOptions(void);
/* Test harness (KHCOM_OPTIONS_TOGGLE): open, or close as the player would. */
void PortUiHarnessToggle(void);
/* Called by the title screen while the options hint should be shown. */
void PortUiTitleHint(void);
/* Short message in the corner of the screen (UTF-8). */
void PortUiNotice(const char* text);
/* Opening movie: show or hide the "A again skips" prompt. */
void PortUiSkipPrompt(int visible);

#endif
