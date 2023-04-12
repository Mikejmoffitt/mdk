#include "util.h"

#include <stdbool.h>
#include <stdio.h>

void origin_for_sp(ConvOrigin origin, int sw, int sh, int *ox, int *oy)
{
	switch (origin)
	{
		case CONV_ORIGIN_LEFT_TOP:      *oy = 0;      *ox = 0;      break;
		case CONV_ORIGIN_CENTER_TOP:    *oy = 0;      *ox = sw / 2; break;
		case CONV_ORIGIN_RIGHT_TOP:     *oy = 0;      *ox = sw - 1; break;
		case CONV_ORIGIN_LEFT_CENTER:   *oy = sh / 2; *ox = 0;      break;
		case CONV_ORIGIN_CENTER_CENTER: *oy = sh / 2; *ox = sw / 2; break;
		case CONV_ORIGIN_RIGHT_CENTER:  *oy = sh / 2; *ox = sw - 1; break;
		case CONV_ORIGIN_LEFT_BOTTOM:   *oy = sh - 1; *ox = 0;      break;
		case CONV_ORIGIN_CENTER_BOTTOM: *oy = sh - 1; *ox = sw / 2; break;
		case CONV_ORIGIN_RIGHT_BOTTOM:  *oy = sh - 1; *ox = sw - 1; break;
	}
}

void render_region(const uint8_t *imgdat, int iw, int ih,
                   int sx, int sy, int sw, int sh)
{
	static const char k_hex[0x10] =
	{
		'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'
	};

	puts("\n");
	for (int y = sy; y < sy + sh; y++)
	{
		for (int x = sx; x < sx + sw; x++)
		{
			const uint8_t px = imgdat[x + (y * iw)] & 0xF;
			const bool grid = ((x % 8) == 0 || (y % 8) == 0);
			const char transparent = (grid) ? '.' : ' ';
			printf("%c ", px ? k_hex[px] : transparent);
		}
		puts("|\n");
	}
	puts("\n");
}


void clip_8x8_tile(uint8_t *imgdat, int iw, int sx, int sy,
                   int limx, int limy, uint8_t *out)
{
	// 8x8 tile, row by row.
	for (int y = 0; y < 8; y++)
	{
		// Source eight pixels from imgdat.
		const int source_y = sy + y;
		uint8_t *line = &imgdat[sx + (source_y * iw)];
		// Walk through two bytes at a time, as the destination data is 4bpp and
		// packs two pixels into one byte.
		for (int x = 0; x < 8; x += 2)
		{
			const int source_x = sx + x;
			uint8_t px[2] = {0, 0};
			// Don't take pixels that exceed boundaries of the sprite clipping
			// region nor the source image data.
			if (source_y < limy)
			{
				if (source_x < limx)
				{
					px[0] = (line[x] & 0xF);
					line[x] = 0;
				}
				if (source_x + 1 < limx)
				{
					px[1] = (line[x + 1] & 0xF);
					line[x + 1] = 0;
				}
			}
			// Write the two pixels as a byte.
			*out++ = (px[0] << 4) | (px[1]);
		}
	}
}
