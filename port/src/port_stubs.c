/*
 * Temporary stand-ins for hand-written ARM code that has no C version yet.
 */
#include <stdint.h>
#include <string.h>

/* crt0 copies its ARM interrupt handler into IWRAM; natively it is unused. */
uint8_t IrqHandler[0x800];

/* Movie codecs (asm/movie_codec.s): self-modifying ARM code, replaced by a
   native decoder in port_movie.c. The labels only need to exist. */
uint8_t MovieVideoCodecStart[0x400];
uint8_t MovieVideoCodecKeyFrame[4];
uint8_t MovieVideoCodecPostProcess[4];
uint8_t MovieVideoCodecConstants[0x60];
uint8_t MovieVideoCodecEnd[4];
uint8_t MovieDeltaCodecStart[0x200];
uint8_t MovieDeltaCodecDecode[4];
uint8_t MovieDeltaCodecOffsets[0x40];
uint8_t MovieAudioCodecStart[0x200];
uint8_t MovieAudioCodecPcm16[4];
uint8_t MovieAudioCodecNull[4];
uint8_t MovieAudioCodecAdpcm[4];
uint8_t MovieAudioCodecAdpcmSteps[0x200];
uint8_t MovieAudioCodecEnd[4];
