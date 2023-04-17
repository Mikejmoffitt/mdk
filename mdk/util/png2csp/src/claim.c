#include "claim.h"

#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

/*

Algorithm strategy:

1) Scan the image from top to bottom until we hit an opaque pixel.
2) Scan left to right, with a test line the max sprite height until a hit.
3) make a 32x32 box from the hit test point.
4) within that 32x32 box, test if any 8px-thick strips may be stripped.
5) claim the resulting image section with the resulting sprite size.

TODO: Make a flag for VRAM usage optimization, where empty tiles are more
      aggressively clipped out at the expense of increasing the sprite count.
*/

static bool empty_test(const uint8_t *imgdat,
                       int iw, int ih,
                       int sx, int sy, int sw, int sh)
{
	// safety check
	int xlim = sx+sw;
	if (xlim > iw) xlim = iw;
	int ylim = sy+sh;
	if (ylim > ih) ylim = ih;
	// test region
	for (int y = sy; y < ylim; y++)
	{
		for (int x = sx; x < xlim; x++)
		{
			if (imgdat[x + (y * iw)] == 0) continue;
			return false;
		}
	}

	return true;
}

// Finds a sprite to clip out of imgdat.
// Returns CLAIM_SIZE_NONE if imgdat is empty.
ClaimSize claim(const uint8_t *imgdat,
                int iw, int ih,
                int sx, int sy, int sw, int sh,
                int *col, int *row)
{
	int max_h = 4;
	int tiles_x = 4;
	int tiles_y = max_h;

	bool satisfied = false;
	do
	{
		// 1) Walk down row by row looking for non-transparent pixel data.
		*row = -1;
		for (int y = sy; y < sy + sh; y++)
		{
			for (int x = sx; x < sx + sw; x++)
			{
				if (imgdat[x + (y * iw)] == 0) continue;
				// Note the row image data was found on, and break out.
				*row = y;
				break;
			}
			// Break out if we are done searching.
			if (*row >= 0) break;
		}
		if (*row < 0) return CLAIM_SIZE_NONE;  // Image is empty.
		//*row = (*row / TSIZE) * TSIZE;

		// 2) We have the top row, but we need to scan within a block to find a
		// viable sprite chunk to extract. Scan rightwards to find a left edge.
		*col = -1;
		const int test_h_px = TSIZE * max_h;
		for (int x = sx; x < sx + sw; x++)
		{
			// As our test column extends TEST_H_PX below the starting line, we must
			// ensure we don't exceed the boundaries of the sprite clipping region
			// or the source image data.
			int ylim = *row + test_h_px;
			if (ylim >= sy + sh) ylim = sy + sh - 1;
			for (int y = *row; y < ylim; y++)
			{
				if (imgdat[x + (y * iw)] == 0) continue;
				// Found it; break out.
				*col = x;
				break;
			}
			// If the column is set, we are done.
			if (*col >= 0) break;
		}
		// Sanity check that something hasn't gone wrong.
		if (*col < 0)
		{
			printf("Unexpectedly empty strip from row %d?\n", *row);
			return CLAIM_SIZE_NONE;
		}
		//*col = (*col / TSIZE) * TSIZE;

		// 3) We now have a 32xh box, at *col, *row. First comes the most obvious
		// optimization which is shrinking it down if it extends outside the frame.
		tiles_x = 4;
		tiles_y = max_h;

		const int tiles_to_right  = ((sx+sw) - (*col - (TSIZE-1))) / TSIZE;
		const int tiles_to_bottom = ((sy+sh) - (*row - (TSIZE-1))) / TSIZE;

		if (tiles_x > tiles_to_right) tiles_x = tiles_to_right;
		if (tiles_y > tiles_to_bottom) tiles_y = tiles_to_bottom;
		if (tiles_x <= 0 || tiles_y <= 0)
		{
			printf("Unexpectedly low tile dimensions %d x %d\n", tiles_x, tiles_y);
			return CLAIM_SIZE_NONE;
		}

		bool reduction_done = false;
		while (!reduction_done)
		{
			reduction_done = true;
			// Try reducing on the right.
			if (tiles_x > 1)
			{
				const int test_x = *col + ((tiles_x - 1) * TSIZE);
				const int test_y = *row;
				const int test_w = TSIZE;
				const int test_h = tiles_y * TSIZE;
				if (empty_test(imgdat, iw, ih, test_x, test_y, test_w, test_h))
				{
					tiles_x--;
					//printf("Reduce x --> %d\n", tiles_x);
					reduction_done = false;
				}
			}
			// Try the bottom too.
			if (tiles_y > 1)
			{
				const int test_x = *col;
				const int test_y = *row + ((tiles_y - 1) * TSIZE);
				const int test_w = tiles_x * TSIZE;
				const int test_h = TSIZE;
				if (empty_test(imgdat, iw, ih, test_x, test_y, test_w, test_h))
				{
					tiles_y--;
					//printf("Reduce y --> %d\n", tiles_y);
					reduction_done = false;
				}
			}
		}

		// 5) Eliminate particularly bad case where only the right and bottom
		//    edges are being utilized due to the previous clip. Re-do with a
		//    reduced maximum height if so.

		if (tiles_x > 1 && tiles_y >= 3)
		{
			satisfied = false;
			int row_util[4];
			for (int ty = 0; ty < tiles_y; ty++)
			{
				row_util[ty] = 0;
				for (int tx = 0; tx < tiles_x; tx++)
				{
					if (!empty_test(imgdat, iw, ih,
					    *col + (TSIZE * tx), *row + (TSIZE * ty),
					    TSIZE, TSIZE))
					{
						row_util[ty]++;
					}
				}
			}

			for (int i = 0; i < tiles_y - 2; i++)
			{
				if (row_util[i] > 1) satisfied = true;
			}
		}
		else
		{
			satisfied = true;
		}

		if (max_h == 1) satisfied = true;
		else if (!satisfied) max_h--;
	} while (!satisfied);

	// 4) Try to reduce the size. As we searched from the top-left, col and
	// row will never be changed.

	switch (tiles_x)
	{
		case 4:
			if (tiles_y == 4) return CLAIM_SIZE_4x4;
			if (tiles_y == 3) return CLAIM_SIZE_4x3;
			if (tiles_y == 2) return CLAIM_SIZE_4x2;
			if (tiles_y == 1) return CLAIM_SIZE_4x1;
			return CLAIM_SIZE_NONE;
		case 3:
			if (tiles_y == 4) return CLAIM_SIZE_3x4;
			if (tiles_y == 3) return CLAIM_SIZE_3x3;
			if (tiles_y == 2) return CLAIM_SIZE_3x2;
			if (tiles_y == 1) return CLAIM_SIZE_3x1;
			return CLAIM_SIZE_NONE;
		case 2:
			if (tiles_y == 4) return CLAIM_SIZE_2x4;
			if (tiles_y == 3) return CLAIM_SIZE_2x3;
			if (tiles_y == 2) return CLAIM_SIZE_2x2;
			if (tiles_y == 1) return CLAIM_SIZE_2x1;
			return CLAIM_SIZE_NONE;
		case 1:
			if (tiles_y == 4) return CLAIM_SIZE_1x4;
			if (tiles_y == 3) return CLAIM_SIZE_1x3;
			if (tiles_y == 2) return CLAIM_SIZE_1x2;
			if (tiles_y == 1) return CLAIM_SIZE_1x1;
			return CLAIM_SIZE_NONE;
		default:
			return CLAIM_SIZE_NONE;
	}
}

int tiles_for_claim_size(ClaimSize size)
{
	switch (size)
	{
		case CLAIM_SIZE_1x1: return 1;
		case CLAIM_SIZE_1x2: return 2;
		case CLAIM_SIZE_1x3: return 3;
		case CLAIM_SIZE_1x4: return 4;
		case CLAIM_SIZE_2x1: return 2;
		case CLAIM_SIZE_2x2: return 4;
		case CLAIM_SIZE_2x3: return 6;
		case CLAIM_SIZE_2x4: return 8;
		case CLAIM_SIZE_3x1: return 3;
		case CLAIM_SIZE_3x2: return 6;
		case CLAIM_SIZE_3x3: return 9;
		case CLAIM_SIZE_3x4: return 12;
		case CLAIM_SIZE_4x1: return 4;
		case CLAIM_SIZE_4x2: return 8;
		case CLAIM_SIZE_4x3: return 12;
		case CLAIM_SIZE_4x4: return 16;
		default:             return 0;
	}
}

int tile_w_for_claim_size(ClaimSize size)
{
	switch (size)
	{
		case CLAIM_SIZE_1x1: return 1;
		case CLAIM_SIZE_1x2: return 1;
		case CLAIM_SIZE_1x3: return 1;
		case CLAIM_SIZE_1x4: return 1;
		case CLAIM_SIZE_2x1: return 2;
		case CLAIM_SIZE_2x2: return 2;
		case CLAIM_SIZE_2x3: return 2;
		case CLAIM_SIZE_2x4: return 2;
		case CLAIM_SIZE_3x1: return 3;
		case CLAIM_SIZE_3x2: return 3;
		case CLAIM_SIZE_3x3: return 3;
		case CLAIM_SIZE_3x4: return 3;
		case CLAIM_SIZE_4x1: return 4;
		case CLAIM_SIZE_4x2: return 4;
		case CLAIM_SIZE_4x3: return 4;
		case CLAIM_SIZE_4x4: return 4;
		default:             return 0;
	}
}

int tile_h_for_claim_size(ClaimSize size)
{
	switch (size)
	{
		case CLAIM_SIZE_1x1: return 1;
		case CLAIM_SIZE_1x2: return 2;
		case CLAIM_SIZE_1x3: return 3;
		case CLAIM_SIZE_1x4: return 4;
		case CLAIM_SIZE_2x1: return 1;
		case CLAIM_SIZE_2x2: return 2;
		case CLAIM_SIZE_2x3: return 3;
		case CLAIM_SIZE_2x4: return 4;
		case CLAIM_SIZE_3x1: return 1;
		case CLAIM_SIZE_3x2: return 2;
		case CLAIM_SIZE_3x3: return 3;
		case CLAIM_SIZE_3x4: return 4;
		case CLAIM_SIZE_4x1: return 1;
		case CLAIM_SIZE_4x2: return 2;
		case CLAIM_SIZE_4x3: return 3;
		case CLAIM_SIZE_4x4: return 4;
		default:             return 0;
	}
}
