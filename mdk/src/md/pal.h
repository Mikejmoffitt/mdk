/* md-toolchain palette support functions
Michael Moffitt 2018-2020 */
#ifndef MD_PAL_H
#define MD_PAL_H

// Functions to read and write to the palette. Color data is cached in a small
// buffer that is queued for transfer during vblank if needed.

#include <stdint.h>

#define PALRGB(r, g, b) (((r) << 1) | ((g) << 5) | ((b) << 9))

// Set a single palette color.
void md_pal_set(uint8_t idx, uint16_t val);

// Caches and schedules a transfer to palette data via DMA.
// Dest refers to palette index (0 - 31).
// Len is in words (one word = one color).
void md_pal_upload(uint8_t dest, const void *source, uint8_t len);

// Internal Use ---------------------------------------------------------------

// Schedules DMA transfers. Called by megadrive_finish().
void md_pal_poll(void);

#endif // MD_PAL_H
