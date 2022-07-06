/* mdk palette support functions
Michael Moffitt 2018-2022 */
#ifndef MD_PAL_H
#define MD_PAL_H

// Functions to read and write to the palette. Color data is cached in a small
// buffer that is queued for transfer during vblank if needed.

#include <stdint.h>

// Note: these are useful for debugging, but I would suggest using external
// binary data for palettes, and uploading them via md_pal_upload().
#ifdef MDK_TARGET_C2
#define PALRGB(r, g, b) (((r)) | ((g) << 4) | ((b) << 8))
#else
#define PALRGB(r, g, b) (((r) << 1) | ((g) << 5) | ((b) << 9))

#endif

// Set a single palette color.
void md_pal_set(uint16_t idx, uint16_t val);

// Caches and schedules a transfer to palette data.
// Dest: palette index (0 - 63 on MD, 0 - 255 on C/C2).
// Source: Pointer to data to copy from. Data is copied immediately to a cache.
// Count: Number of palette entries to copy (1 = one word = one color).
void md_pal_upload(uint16_t dest, const void *source, uint16_t count);

// Select between banks 0 - 3 for sprites and backgrounds. (System C/C2 only).
void md_pal_set_bg_bank(uint16_t bank);
void md_pal_set_spr_bank(uint16_t bank);

// Internal Use ---------------------------------------------------------------

// Schedules DMA transfers. Called by megadrive_finish().
void md_pal_poll(void);

void md_pal_init(void);

#endif // MD_PAL_H
