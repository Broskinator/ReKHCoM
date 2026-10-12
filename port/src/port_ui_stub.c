#include "port/port_ui.h"
void PortUiInit(void) {}
void PortUiDraw(uint32_t* frame, int width, int height) { (void)frame; (void)width; (void)height; }
int PortUiHandleEvent(const void* ev) { (void)ev; return 0; }
int PortUiActive(void) { return 0; }
void PortUiOpenOptions(void) {}
