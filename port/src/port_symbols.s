/* Fixed addresses the game expects from the BIOS and the original linker
   script. Everything here lies inside the mapped GBA work RAM. */
	.globl _gEwramHeapStart
	.set _gEwramHeapStart, 0x02000000
	.globl _gIwramHeapStart
	.set _gIwramHeapStart, 0x03000000
	.globl _gSoundInfoPtr
	.set _gSoundInfoPtr, 0x03007FF0
	.globl _gIntrCheck
	.set _gIntrCheck, 0x03007FF8
