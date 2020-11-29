/* md-toolchain palette support functions
Michael Moffitt 2018-2020 */
#ifndef MD_PAL_H
#define MD_PAL_H

#include <stdint.h>

#define PALRGB(r, g, b) (((r) << 1) | ((g) << 5) | ((b) << 9))

// Set a single palette color.
void pal_set(uint8_t idx, uint16_t val);

// Fetch a single palette color.
uint16_t pal_get(uint8_t idx);

// Upload a palette of arbitrary size via DMA.
// Dest refers to palette slot (true VRAM destination * 2)
// Len is in words (1 word per palette entry)
void pal_upload(uint8_t dest, const void *source, uint8_t len);

#endif // MD_PAL_H
