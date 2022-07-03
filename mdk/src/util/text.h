// mdk print utility functions
// Michael Moffitt 2018-2022
//
// These functions are meant to be more instructional than practical. They are
// not particularly flexible, so depending on your game's needs, it may be
// sensible to not use these.

#ifndef UTIL_TEXT_H
#define UTIL_TEXT_H

#include "md/megadrive.h"

// Load the font graphics and palette.
// Pass 0 as font_chr to skip uploading the text graphics to
// VRAM and only redirect the base tile index.
// Pass 0 as font_pal to redirect the base palette without
// overwriting the contents of that palette line.
void text_init(const unsigned char *font_chr, uint16_t font_len,
               uint16_t vram_pos,
               const unsigned char *font_pal, uint16_t pal_line);

// Print a string s on a specified plane at coordinates x, y
void text_puts(VdpPlane plane, uint16_t x, uint16_t y, const char *s);

#endif // UTIL_TEXT_H
