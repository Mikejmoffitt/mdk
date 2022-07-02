#ifndef PAL_SYSTEMC_H
#define PAL_SYSTEMC_H

// Functions to read and write to the palette. Color data is cached in a small
// buffer that is queued for transfer during vblank if needed.

#include <stdint.h>

#define PAL_SYSTEMC_RGB(r, g, b) (((r)) | ((g) << 5) | ((b) << 10))

// Set a single palette color.
void pal_systemc_set(uint8_t bank, uint8_t idx, uint16_t val);

// Caches and schedules a transfer to palette data.
// Bank 0-3 may be used, regardless of which is currently selected.
// Dest refers to palette index (0 - 255).
// Len is in words (one word = one color).
void pal_systemc_upload(uint8_t bank, uint8_t dest,
                        const uint16_t *source, uint16_t len);

// Internal Use ---------------------------------------------------------------

// Schedules transfers. Called by systemc_finish().
void pal_systemc_poll(void);

#endif  // PAL_SYSTEMC_H
