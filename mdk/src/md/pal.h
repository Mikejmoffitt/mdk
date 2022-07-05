/* mdk palette support functions
Michael Moffitt 2018-2022 */
#ifndef MD_PAL_H
#define MD_PAL_H

// Functions to read and write to the palette. Color data is cached in a small
// buffer that is queued for transfer during vblank if needed.

#include <stdint.h>

#ifdef MDK_TARGET_C2
#define PALRGB(r, g, b) (((r)) | ((g) << 5) | ((b) << 10))
#else
#define PALRGB(r, g, b) (((r) << 1) | ((g) << 5) | ((b) << 9))

#endif

// Set a single palette color.
void md_pal_set(uint8_t idx, uint16_t val);

// Caches and schedules a transfer to palette data.
// Dest: palette index (0 - 63 on MD, 0 - 255 on C/C2).
// Source: Pointer to data to copy from. Data is copied immediately to a cache.
// Count: Number of palette entries to copy (1 = one word = one color).
void md_pal_upload(uint16_t dest, const void *source, uint16_t count);

// System C/C2 only: Set upper bits of CRAM indices for sprites and backgrounds.
// For usage resembling the MD (with BG and sprites shared) set both to zero.
// For typical System C usage, set bg_pos to 0, and spr_pos to 3.
// The default is 0/0.
void md_pal_set_sysc_map(uint16_t bg_pos, uint16_t spr_pos);

// Internal Use ---------------------------------------------------------------

// Schedules DMA transfers. Called by megadrive_finish().
void md_pal_poll(void);

#endif // MD_PAL_H
