#ifndef GUARD_PORT_STATE_H
#define GUARD_PORT_STATE_H

#include <stddef.h>

/* Save states (port_state.c). */
typedef void (*PortStateVisitor)(void* data, size_t size);

extern int gPortInMainLoop; /* the current frame wait comes from AgbMain */
extern int gPortStateSlot;

const char* PortStatePath(int slot);
void PortStateRequestSave(const char* path); /* NULL: current slot */
void PortStateRequestLoad(const char* path);
void PortStateService(void);

/* Each module exposes the hardware state it emulates. */
void PortDmaVisitState(PortStateVisitor visit);
void PortPpuVisitState(PortStateVisitor visit);
void PortAudioVisitState(PortStateVisitor visit);
void PortAudioReset(void);

#endif
