// md-toolchain example main.c
// Michael Moffitt 2018
//
// This main shows a simple "hello world" demo.

// megadrive.h is an umbrella for all headers in src/md. Specific modules like
// md/vdp.h do not need to be individually included. However, utility funcitons
// are not included, as they are not core support functions.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"
#include "md/psg.h"

#include "md/adpcm.h"

#include "res.h"

static const uint16_t tile_vram_addr = 0x0;

static void draw_interface(void)
{
	text_puts(VDP_PLANE_A, 1, 1, "Hello System C2");
	text_puts(VDP_PLANE_A, 1, 3,  "       0123456789ABCDEF");
	text_puts(VDP_PLANE_A, 1, 4,  "BG  0");
	text_puts(VDP_PLANE_A, 1, 5,  "BG  1");
	text_puts(VDP_PLANE_A, 1, 6,  "BG  2");
	text_puts(VDP_PLANE_A, 1, 7,  "BG  3");
	text_puts(VDP_PLANE_A, 1, 8,  "SP  0");
	text_puts(VDP_PLANE_A, 1, 9,  "SP  1");
	text_puts(VDP_PLANE_A, 1, 10, "SP  2");
	text_puts(VDP_PLANE_A, 1, 11, "SP  3");

	// Center the screen at 0, 0.
	static uint16_t x_scroll, y_scroll;
	x_scroll = 0;
	y_scroll = 0;
	md_dma_transfer_vram(VRAM_HSCR_BASE_DEFAULT, &x_scroll, 1, 2);
	md_dma_transfer_vsram(0, &y_scroll, 1, 2);
}

static void draw_bg_color_bars(void)
{
	md_vdp_set_autoinc(2);
	static const uint16_t start_tile_x = 8;
	static const uint16_t start_tile_y = 4;
	static const uint16_t start_tile_no = tile_vram_addr / 32;

	for (uint16_t i = 0; i < 4; i++)
	{
		const uint16_t pal = i % 4;
		const uint16_t prio = (i / 4) % 2;
		const uint16_t plane_a_base = md_vdp_get_plane_base(VDP_PLANE_A);
		md_vdp_set_addr(plane_a_base +
		                (2 * start_tile_x) +
		                (2 * (start_tile_y + i) * md_vdp_get_plane_width()));
		for (uint16_t j = 0; j < 16; j++)
		{
			const uint16_t tile = start_tile_no + j;
			md_vdp_write(VDP_ATTR(tile, 0, 0, pal, prio));
		}
	}
}

static void draw_sprite_color_bars(void)
{
	static const uint16_t start_tile_x = 8;
	static const uint16_t start_tile_y = 8;
	static const uint16_t start_tile_no = tile_vram_addr / 32;

	for (uint16_t i = 0; i < 4; i++)
	{
		const uint16_t pal = i % 4;
		const uint16_t prio = (i / 4) % 2;
		for (uint16_t j = 0; j < 4; j++)
		{
			const uint16_t x = (start_tile_x * 8) + (j * 32);
			const uint16_t y = (start_tile_y + i) * 8;
			const uint16_t tile = start_tile_no + (j * 4);
			const uint16_t attr = SPR_ATTR(tile, 0, 0, pal, prio);
			const uint16_t size = SPR_SIZE(4, 1);
			md_spr_put(x, y, attr, size);
		}
	}
}

static void initialize_resources(void)
{
	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);
	md_dma_transfer_vram(tile_vram_addr, res_color_blocks_bin,
	                     sizeof(res_color_blocks_bin) / 2, 2);

	// Generate color bars spanning the four palettes.
	for (uint16_t i = 0; i < 16; i++)
	{
		md_pal_set(i, PALRGB(i, i, i));
		md_pal_set(i+(1*16), PALRGB(i, 0, 0));
		md_pal_set(i+(2*16), PALRGB(i, i, 0));
		md_pal_set(i+(3*16), PALRGB(0, i, 0));
		md_pal_set(i+(16*16), PALRGB(0, i, i));
		md_pal_set(i+(17*16), PALRGB(0, 0, i));
		md_pal_set(i+(18*16), PALRGB(i, 0, i));
		md_pal_set(i+(19*16), PALRGB(i, i/2, 0));
	}

	md_pal_set_bg_bank(0);
}

void main(void)
{
	megadrive_init();
	initialize_resources();
	draw_interface();
	draw_bg_color_bars();

	// It makes a bunch of noise.
	md_psg_vol(0, 0);
	md_psg_vol(1, 0);
	md_psg_vol(2, 0);
	md_adpcm_play(0);
	static uint16_t period = 0;
	while (1)
	{
		md_psg_pitch(2, 2 * (0x400 - period));
		md_psg_pitch(1, 0x400 - period);
		md_psg_pitch(0, period++);
		draw_sprite_color_bars();
		megadrive_finish();
	}
}
