#ifndef GUARD_PORT_AUDIO_H
#define GUARD_PORT_AUDIO_H

#include <stdint.h>

/* Samples per frame the mixer can produce (15768 Hz x 4 / 59.7 Hz < 1100). */
#define PORT_MIX_MAX 1200

/* Output of the m4a mixer for one frame, in units of 1/256 of an 8-bit
   DirectSound sample. */
typedef struct PortMixBuffer {
    int32_t right[PORT_MIX_MAX];
    int32_t left[PORT_MIX_MAX];
    int count;
    int rate;
    int ready;
    int oversample;   /* mixing rate multiplier (1 = hardware rate) */
    float musicGain;
    float sfxGain;
} PortMixBuffer;

extern PortMixBuffer gPortMix;

#endif
