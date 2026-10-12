/*
 * Temporary stand-ins for hand-written ARM code that has no C version yet.
 */
#include <stdint.h>
#include <string.h>

/* crt0 copies its ARM interrupt handler into IWRAM; natively it is unused. */
uint8_t IrqHandler[0x800];
