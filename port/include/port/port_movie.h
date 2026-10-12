#ifndef GUARD_PORT_MOVIE_H
#define GUARD_PORT_MOVIE_H
#include <stdint.h>
#include <stddef.h>
int PortMovieDecodeKey(uint16_t*, uint32_t, uint32_t, const uint8_t*, size_t);
void PortMoviePostProcess(uint16_t*, uint32_t, uint32_t);
int PortMovieDecodeDelta(const uint16_t*, uint16_t*, uint32_t, uint32_t, const uint8_t*, size_t);
int PortMovieDecodeAudio(const uint8_t*, size_t, int8_t*, uint32_t, unsigned);
#endif
