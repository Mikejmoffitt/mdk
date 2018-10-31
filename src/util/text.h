// md-toolchain print utility functions
// Michael Moffitt 2018
//
// These functions are meant to be more instructional than practical. They are
// not particularly flexible, so depending on your game's needs, it may be
// sensible to not use these.

#ifndef TEXT_H
#define TEXT_H

#define TEXT_VRAM_NO_LOAD 0x0001
#define TEXT_PAL_NO_LOAD  0x1000

#include "md/megadrive.h"

// Load the font graphics (res/gfx/font.bin) and palette (res/gfx/font.pal)
// OR vram_pos with TEXT_VRAM_NO_LOAD to skip uploading the text graphics to
// VRAM and only redirect the base tile index.
// OR pal_lne with TEXT_PAL_NO_LOAD to redirect the base palette without
// overwriting the contents of that palette line.
void text_init(uint16_t vram_pos, uint16_t pal_line);

// Print a string s on a specified plane at coordinates x, y
void text_puts(VdpPlane plane, uint16_t x, uint16_t y, const char *s);

#endif // TEXT_H
