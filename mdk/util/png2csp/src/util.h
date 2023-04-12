#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#include "types.h"

// Small utility functions used in sprite extraction. Arguments are concise,
// so a generalized description has been provided below:
// imgdat: uint8_t array of sprite sheet bitmap data; one pixel is one byte.
// iw, ih: width and height of source image data (the sprite sheet bitmap).
// sw, sh: sprite frame size to be extracted.
// sx, sy: top-left origin point of sprite frame being extracted from imgdat.
// ox, oy: origin point considered the "center" of the extracted sprite.

// Sets ox and oy based on the origin mode and sprite frame size.
void origin_for_sp(ConvOrigin origin, int sw, int sh, int *ox, int *oy);

// (debug) Renders a region of imgdat as terminal output.
void render_region(const uint8_t *imgdat, int iw, int ih,
                   int sx, int sy, int sw, int sh);

// Takes the 8x8 tile from imgdat and places it in the appropriate 4bpp format
// into out. *** The data is erased from imgdat as it is taken. ***
// It is a given that imgdat is large enough for the indicated region.
// Data exceeding sw and sh is excluded.
void clip_8x8_tile(uint8_t *imgdat, int iw, int sx, int sy,
                   int limx, int limy, uint8_t *out);
#endif  // UTIL_H
