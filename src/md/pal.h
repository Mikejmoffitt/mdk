/* md-toolchain palette support functions
Michael Moffitt 2018 */
#ifndef PAL_H
#define PAL_H

#include <stdint.h>

// Set a single palette color.
void pal_set(uint8_t idx, uint16_t val);

// Fetch a single palette color.
uint16_t pal_get(uint8_t idx);

// Upload a palette of arbitrary size via DMA.
// Dest refers to palette slot (true VRAM destination * 2)
// Len is in words (1 word per palette entry)
void pal_upload(uint8_t dest, void *source, uint8_t len);

#endif // PAL_H
