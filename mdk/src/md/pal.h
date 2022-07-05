/* mdk palette support functions
Michael Moffitt 2018-2022 */
#ifndef MD_PAL_H
#define MD_PAL_H

// Functions to read and write to the palette. Color data is cached in a small
// buffer that is queued for transfer during vblank if needed.

// RGB333 and RGB555 functions exist to allow for the use of color data intended
// for one platform on another. If an RGB333 function is provided color data
// for the Mega Drive, but the target is System C2, the palette data will be
// translated to the native RGB555 format. The reverse is true as well.

#include <stdint.h>

#ifdef MDK_TARGET_C2
#define PALRGB(r, g, b) (((r)) | ((g) << 5) | ((b) << 10))
#else
#define PALRGB(r, g, b) (((r) << 1) | ((g) << 5) | ((b) << 9))

#endif

// Set a single palette color.
void md_pal_set(uint8_t idx, uint16_t val);
void md_pal_set_rgb33(uint8_t idx, uint16_t val);
void md_pal_set_rgb55(uint8_t idx, uint16_t val);

// Caches and schedules a transfer to palette data.
// Dest: palette index (0 - 63 on MD, 0 - 255 on C/C2).
// Source: Pointer to data to copy from. Data is copied immediately to a cache.
// Count: Number of palette entries to copy (1 = one word = one color).
// No transformation is done to the color data.
void md_pal_upload(uint16_t dest, const void *source, uint16_t count);

// Caches and schedules a transfer to palette data.
// Dest: palette index (0 - 63 on MD, 0 - 255 on C/C2).
// Source: Pointer to data to copy from. Data is copied immediately to a cache.
// Count: Number of palette entries to copy (1 = one word = one color).
// If used on System C2, the palette data is converted to native RGB555.
void md_pal_upload_rgb333(uint16_t dest, const void *source, uint16_t count);

// Caches and schedules a transfer to palette data.
// Dest: palette index (0 - 63 on MD, 0 - 255 on C/C2).
// Source: Pointer to data to copy from. Data is copied immediately to a cache.
// Count: Number of palette entries to copy (1 = one word = one color).
// If used on Mega Drive, the palette data is converted to native RGB333.
void md_pal_upload_rgb555(uint16_t dest, const void *source, uint16_t count);

// Internal Use ---------------------------------------------------------------

// Schedules DMA transfers. Called by megadrive_finish().
void md_pal_poll(void);

#endif // MD_PAL_H
