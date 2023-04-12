// png2cspr
// (c) 2023 Michael Moffitt
//
// Utility to convert a spritesheet PNG into data for use with MDK's composite
// sprite drawing routines.
//
// Usage:
//
//     png2cspr spritesheet.png frame_width frame_height output_name <origin>

/*

The goal is to allow for a sprite to be either drawn using DMA'd data as needed
for each frame, or with all of the necessary tile data already uploaded to VRAM.
This format is designed to accomodate either use case.

Composite sprites start from one tile data offset and are expected to use a
continiguous line of tile data. While this may result in some duplication of
image data, as of writing storage space is easy to accomodate on the megadrive
while the feeble CPU and pedestrian DMA cannot be surmounted. It is faster to
allow the transfer of all the tile data in one DMA, so this system was chosen.

Binary blob format:
-------------------
16  bank name (null-terminated C string)
32  palette data
2   sprite ref count (available images)
4   spr list offset (from start of file)
4   tile data offset (from start of file)
4   reserved
... ref list
... spr list data (length variable; referened by header)
... tile data (length variable; referenced by header)


ref def - a single metasprite frame
-----------------------------------
2   spr quantity used
2   spr list index * sizeof(spr def)
2   tile data start offset (within tile data block)
2   tile count * 16 (words, for DMA, pre-multiplied by VDP tile size)


spr def - a single hardware sprite directive
--------------------------------------------
2   relative Y offset (signed word)
1   sprite size (4 bits, shifted left by 4 to match hardware def)
1   reserved (set to 0)
2   attributes (flags, tile id)
2   relative X offset (signed word)

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lodepng.h"

#include "types.h"
#include "records.h"
#include "util.h"
#include "constants.h"

#include "claim.h"

static void show_usage(const char *prog_name)
{
	printf("Usage: %s sprites.png w h outname bankname <o>\n", prog_name);
	printf("      w: Width of sprite within spritesheet (decimal or hex)\n");
	printf("      h: Height of sprite within spritesheet (decimal or hex)\n");
	printf("outname: Output filename path.\n");
	printf("sprname: Sprite developer-legible name.\n");
	printf("      o: Origin (XY, where both characters form an argument\n");
	printf("         t/l: top/left\n");
	printf("           c: center\n");
	printf("         b/r: bottom/right\n");
	printf("         e.g. \"lt\" uses the left-top as the origin.\n");
	printf("\n");
	printf("As an example, for the following command\n\n");
	printf("    %s player.png 32 48 out/player x cb\n", prog_name);
	printf("\n");
	printf("'player.png' is loaded, and out/player.csp will be emitted.\n");
	printf("\n");
	printf("'player.png' is chopped into 32 x 48 composite sprites, with the\n");
	printf("center-bottom of the frame as the origin point (0, 0).\n");
}

static bool check_arg_sanity(int argc, char **argv)
{
	if (argc < 6)
	{
		show_usage(argv[0]);
		return false;
	}

	const int frame_w = strtoul(argv[2], NULL, 0);
	const int frame_h = strtoul(argv[3], NULL, 0);
	if (frame_w < 0 || frame_h < 0)
	{
		printf("Invalid frame size %d x %d\n", frame_w, frame_h);
		return false;
	}
	return true;
}

// Free after usage. NULL on error.
static uint8_t *load_png_data(const char *fname,
                              unsigned int *png_w, unsigned int *png_h,
                              LodePNGState *state)
{
	uint8_t *png;
	uint8_t *ret;
	// First load the file into memory.
	size_t fsize;
	int error = lodepng_load_file(&png, &fsize, fname);
	if (error)
	{
		printf("LodePNG error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}

	// The image is decoded as an 8-bit indexed color PNG; we don't want any
	// conversion to take place.
	lodepng_state_init(state);
	state->info_raw.colortype = LCT_PALETTE;
	state->info_raw.bitdepth = 8;
	error = lodepng_decode(&ret, png_w, png_h, state, png, fsize);
	if (error)
	{
		printf("LodePNG error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}

	printf("Loaded \"%s\": %d x %d\n", fname, *png_w, *png_h);
	return ret;
}

// Takes sprite data from imgdat and generates XSP entry data for it.
// Adds to the PCG, FRM, and REF files as necessary.
static void chop_sprite(uint8_t *imgdat, int iw, int ih, ConvOrigin origin,
                        int sx, int sy, int sw, int sh)
{
	const int base_spr_index = record_get_spr_count();
	int spr_in_sprite = 0;  // Hardware sprites used to draw this csprite.
	const int base_tile_index = record_get_tile_count();
	int tiles_for_sprite = 0;

	int ox, oy;
	origin_for_sp(origin, sw, sh, &ox, &oy);

	// DEBUG
	// TODO: Verbose #define
	//render_region(imgdat, iw, ih, sx, sy, sw, sh);

	int clip_x, clip_y;
	int last_vx = 0;
	int last_vy = 0;

	//
	// Claim an area from which a sprite can be extracted.
	//
	ClaimSize claim_size;
	while ((claim_size = claim(imgdat, iw, ih, sx, sy, sw, sh, &clip_x, &clip_y)))
	{
		spr_in_sprite++;
		//
		// Store the PCG data.
		//

		uint8_t tile_data[TBYTES * 4 * 4];  // Up to one 32x32px sprite (16 tiles).
		int tiles_clipped = 0;

		const int tiles_w = tile_w_for_claim_size(claim_size);
		const int tiles_h = tile_h_for_claim_size(claim_size);

		//printf("F%02d Sp%02d: %d, %d %dx%d\n", record_get_ref_count(), record_get_spr_count(), clip_x, clip_y, tiles_w, tiles_h);

		const int limx = sx + sw;
		const int limy = sy + sh;

		for (int tx = 0; tx < tiles_w; tx++)
		{
			for (int ty = 0; ty < tiles_h; ty++)
			{
				clip_8x8_tile(imgdat, iw,
				              clip_x + (tx * TSIZE),
				              clip_y + (ty * TSIZE),
				              limx, limy,
				              &tile_data[TBYTES * tiles_clipped]);
				tiles_clipped++;
			}
		}

		record_tiles(tile_data, tiles_clipped);
//		render_region(imgdat, iw, ih, sx, sy, sw, sh);

		//
		// Record the hardware sprite entry.
		//
		const int vx = ((clip_x % sw) - ox);
		const int vy = ((clip_y % sh) - oy);

		record_spr(vx - last_vx, vy - last_vy, tiles_w, tiles_h,
		           tiles_for_sprite);

		last_vx = vx;
		last_vy = vy;

		//
		// Offset tiles (within base tile index)
		//
		tiles_for_sprite += tiles_clipped;
	}

	//
	// Register ref data.
	//
	printf("F%02d: %d sprites, %d tiles\n", record_get_ref_count(), spr_in_sprite, tiles_for_sprite);
	record_ref(spr_in_sprite, base_spr_index, base_tile_index, tiles_for_sprite);
}

static void handle_palette(const LodePNGState *state)
{
	uint16_t pal[PALSIZE];
	memset(pal, 0, sizeof(pal));
	// We don't want the first palette index as it is always transparent.
	for (int i = 1; i < PALSIZE; i++)
	{
		// LodePNG palette data is sets of four bytes in RGBA order.
		const int offs = i * 4;
		const uint8_t r = state->info_png.color.palette[offs + 0];
		const uint8_t g = state->info_png.color.palette[offs + 1];
		const uint8_t b = state->info_png.color.palette[offs + 2];

		// MD and C/C2 format. MD will ignore extra C/C2 precision.
		const uint16_t entry = ((r >> 3 & 1) ? 0x1000 : 0x0000) |
		                       ((g >> 3 & 1) ? 0x2000 : 0x0000) |
		                       ((b >> 3 & 1) ? 0x4000 : 0x0000) |
		                       (((r >> 4) & 0x0F)) |
		                       (((g >> 4) & 0x0F) << 4) |
		                       (((b >> 4) & 0x0F) << 8);
		pal[i] = entry;
	}
	record_palette(pal);
}

int main(int argc, char **argv)
{
	if (!check_arg_sanity(argc, argv)) return 0;
	
	// User parameters.
	const char *fname = argv[1];
	const int frame_w = strtoul(argv[2], NULL, 0);
	const int frame_h = strtoul(argv[3], NULL, 0);
	const char *outname = argv[4];
	const char *bankname = argv[5];
	const ConvOrigin origin = conv_origin_from_args(argc, argv);

	// Prepare the PNG image.
	unsigned int png_w = 0;
	unsigned int png_h = 0;
	LodePNGState state;
	uint8_t *imgdat = load_png_data(fname, &png_w, &png_h, &state);
	if (!imgdat) return -1;
	if (frame_w > png_w || frame_h > png_h)
	{
		printf("Frame size (%d x %d) exceed source image (%d x %d)\n",
		       frame_w, frame_h, png_w, png_h);
		goto finished;
	}

	record_init();
	handle_palette(&state);

	// Chop sprites out of the image data.
	const int sprite_rows = png_h / frame_h;
	const int sprite_columns = png_w / frame_w;
	for (int y = 0; y < sprite_rows; y++)
	{
		for (int x = 0; x < sprite_columns; x++)
		{
			chop_sprite(imgdat, png_w, png_h, origin,
			            x * frame_w, y * frame_h, frame_w, frame_h);
		}
	}

	printf("%i Frames.\n", record_get_ref_count());
	printf("%d Sprites.\n", record_get_spr_count());
	printf("%d Tiles.\n", record_get_tile_count());

	record_complete(bankname, outname);

finished:
	free(imgdat);

	return 0;
}
