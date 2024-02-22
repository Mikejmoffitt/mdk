// mdk composite sprite demo
// Michael Moffitt 2023
//
// This file shwos how to draw composite sprites made with png2cspr.

#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

#include <stdint.h>
#include <stdbool.h>

void main(void)
{
	megadrive_init();
	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);

	text_puts(VDP_PLANE_A, 1, 1, "==== MDK COMPOSITE SPRITE SAMPLE! ====");

	md_cspr_upload_pal(res_count_csp, 1);
	md_cspr_upload_pal(res_cirno_csp, 2);
	md_cspr_upload_pal(res_sonic_csp, 3);

	const char *kinstructions[] =
	{
		"BUTTON A: Select Sprite",
		"BUTTON B: Toggle V flip",
		"BUTTON C: Toggle H flip",
		"BUTTON X: Toggle priority",
		"BUTTON Y: Change palette",
		"BUTTON Z: Test \"fast\" routine",
		"   D-PAD: Move Sprite",
		"   START: Pause / Resume",
	};

	for (uint16_t i = 0; i < ARRAYSIZE(kinstructions); i++)
	{
		const int16_t print_y = 28 - ARRAYSIZE(kinstructions) + i - 1;
		text_puts(VDP_PLANE_A, 1, print_y, kinstructions[i]);
	}

	// The sprites.
	CSprParam cspr_param[16];
	// First one will show the count animation.
	cspr_param[0].cspr_data = res_count_csp;
	cspr_param[0].vram_base = 0x1000;
	cspr_param[0].x = 282;
	cspr_param[0].y = 52;
	cspr_param[0].attr = SPR_ATTR(0, 0, 0, 1, 0);
	cspr_param[0].frame = 0;
	cspr_param[0].use_dma = true;
	// Second is sonic.
	cspr_param[1].cspr_data = res_sonic_csp;
	cspr_param[1].vram_base = 0x1800;
	cspr_param[1].x = 32;
	cspr_param[1].y = 128;
	cspr_param[1].attr = SPR_ATTR(0, 0, 0, 3, 0);
	cspr_param[1].frame = 0;
	cspr_param[1].use_dma = true;
	// Populate with data for the cirnos.
	for (uint16_t i = 2; i < ARRAYSIZE(cspr_param); i++)
	{
		cspr_param[i].cspr_data = res_cirno_csp;
		cspr_param[i].vram_base = 0x1C00;
		cspr_param[i].x = (18 * i);
		cspr_param[i].y = 10+(12 * i);
		cspr_param[i].attr = SPR_ATTR(0, 0, 0, 2, 0);
		cspr_param[i].frame = i % md_cspr_get_frame_count(res_cirno_csp);
		cspr_param[i].use_dma = false;
	}

	// Cirno isn't using DMA, so tiles should be uploaded prior to drawing.
	md_cspr_upload_tiles(res_cirno_csp, 0x1C00);

	uint16_t frame = 0;
	uint16_t selected_spr = 0;
	bool anim_pause = false;
	while (true)
	{
		frame++;

		// Animate and render
		if (!anim_pause)
		{
			for (uint16_t i = 0; i < ARRAYSIZE(cspr_param); i++)
			{
				if (frame % 8 == 0) cspr_param[i].frame++;
				const uint16_t max = md_cspr_get_frame_count(cspr_param[i].cspr_data);
				if (cspr_param[i].frame >= max)
				cspr_param[i].frame = 0;
			}
		}

		// The A button changes which sprite is under control.
		if (g_md_pad_pos[0] & BTN_A)
		{
			selected_spr++;
			if (selected_spr >= ARRAYSIZE(cspr_param)) selected_spr = 0;
		}

		// Start is used to pause and resume.
		if (g_md_pad_pos[0] & BTN_START) anim_pause = !anim_pause;
		// Movement of selected sprite
		if (g_md_pad[0] & BTN_UP) cspr_param[selected_spr].y--;
		if (g_md_pad[0] & BTN_DOWN) cspr_param[selected_spr].y++;
		if (g_md_pad[0] & BTN_LEFT) cspr_param[selected_spr].x--;
		if (g_md_pad[0] & BTN_RIGHT) cspr_param[selected_spr].x++;
		// Flipping of sprites with B and C buttons
		if (g_md_pad_pos[0] & BTN_C)
			cspr_param[selected_spr].attr ^= SPR_ATTR(0, 1, 0, 0, 0);
		if (g_md_pad_pos[0] & BTN_B)
			cspr_param[selected_spr].attr ^= SPR_ATTR(0, 0, 1, 0, 0);
		// Various attr bits with X and Y
		if (g_md_pad_pos[0] & BTN_X)
			cspr_param[selected_spr].attr ^= SPR_ATTR(0, 0, 0, 0, 1);
		if (g_md_pad_pos[0] & BTN_Y)
		{
			int16_t pal = (cspr_param[selected_spr].attr >> 13) & 0x03;
			pal = (pal + 1) & 0x03;
			cspr_param[selected_spr].attr &= ~SPR_ATTR(0, 0, 0, 3, 0);
			cspr_param[selected_spr].attr |= SPR_ATTR(0, 0, 0, pal, 0);
		}

		// Use the MODE button to test the "fast" drawing function..
		for (uint16_t i = 0; i < ARRAYSIZE(cspr_param); i++)
		{
			if (g_md_pad[0] & BTN_Z) md_cspr_put_st_fast(&cspr_param[i]);
			else md_cspr_put_st(&cspr_param[i]);
		}
		megadrive_finish();
	}
}
