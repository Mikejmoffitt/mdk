// mdk composite sprite demo
// Michael Moffitt 2023
//
// This file shwos how to draw composite sprites made with png2cspr.
//
// This is not fast, and is instead a reference.

#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct CSprRef
{
	uint16_t spr_count;
	uint16_t spr_list_offset;  // Added to spr_list_offset.
	uint16_t tile_src_offset;  // Added to tile_data_offset.
	uint16_t tile_words;
} CSprRef;

typedef struct CSprSprite
{
	int16_t dy;
	uint16_t size;
	uint16_t tile;
	int16_t dx;
	int16_t flip_dy;
	uint16_t reserved[2];
	int16_t flip_dx;
} CSprSprite;

static inline uint16_t cspr_get_ref_count(const uint8_t *cspr)
{
	const uint16_t ref_count = *(uint16_t *)(&cspr[0x30]);
	return ref_count;
}

void draw_cspr(const uint8_t *cspr, uint16_t vram_base, uint16_t frame, int16_t x, int16_t y, uint16_t base_attr, bool use_dma)
{
	const uint16_t ref_count = *(uint16_t *)(&cspr[0x30]);
	const uint32_t spr_list_offset = *(uint32_t *)(&cspr[0x32]);
	const uint32_t tile_data_offset = *(uint32_t *)(&cspr[0x36]);

	const CSprRef *refs = (const CSprRef *)(&cspr[0x40]);
	const uint8_t *tile_data = cspr + tile_data_offset;

	// Reference data for the frame to be drawn.
	const CSprRef *ref = &refs[frame];

	// Set up the DMA based on tile data.
	if (use_dma)
	{
		if (ref->tile_words > 0)
		{
			md_dma_transfer_vram(vram_base, tile_data + ref->tile_src_offset,
			                     ref->tile_words, 2);
		}
	}
	else  // Or offset sprite tile indicator.
	{
		vram_base += ref->tile_src_offset;
	}

	// Draw each sprite.
	const CSprSprite *spr = (const CSprSprite *)(cspr + spr_list_offset + ref->spr_list_offset);
	const bool hflip = base_attr & 0x0800;
	const bool vflip = base_attr & 0x1000;

	x -= 128;
	y -= 128;

	for (uint16_t i = 0; i < ref->spr_count; i++)
	{
		const uint16_t attr = ((vram_base / 32) + spr->tile) | base_attr;
		const uint8_t size = spr->size >> 8;

		x += hflip ? spr->flip_dx : spr->dx;
		y += vflip ? spr->flip_dy : spr->dy;

		md_spr_put(x, y, attr, size);
		spr++;
	}
}

void set_cspr_tiles(const uint8_t *cspr, uint16_t vram_base)
{
	const uint32_t tile_data_offset = *(uint32_t *)(&cspr[0x36]);
	const uint8_t *tile_data = cspr + tile_data_offset;
	const uint16_t tile_count = *(uint16_t *)(&cspr[0x3A]);
	if (tile_count == 0) return;
	md_dma_transfer_vram(vram_base, tile_data, tile_count * 16, 2);
}

void set_cspr_pal(const uint8_t *cspr, int16_t line)
{
	const uint16_t *pal_data = (const uint16_t *)(&cspr[16]);
	md_pal_upload(line*16, pal_data, 16);
}

void main(void)
{
	megadrive_init();
	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);

	text_puts(VDP_PLANE_A, 4, 1, res_count_csp);
	set_cspr_pal(res_count_csp, 1);

	text_puts(VDP_PLANE_A, 24, 1, res_cirno_csp);
	set_cspr_tiles(res_cirno_csp, 0x2000);
	set_cspr_pal(res_cirno_csp, 2);

	CSprParam cspr_param[32];
	for (uint16_t i = 0; i < ARRAYSIZE(cspr_param); i++)
	{
		cspr_param[i].cspr_data = res_cirno_csp;
		cspr_param[i].vram_base = 0x2000;
		cspr_param[i].x = (18 * i);
		cspr_param[i].y = (12 * i);
		cspr_param[i].attr = SPR_ATTR(0, 0, 0, 2, 0);
		cspr_param[i].prio = 0;
		cspr_param[i].use_dma = false;
	}
	// First one will show count instead.
	cspr_param[0].cspr_data = res_count_csp;
	cspr_param[0].vram_base = 0x1000;
	cspr_param[0].x = 32;
	cspr_param[0].y = 32;
	cspr_param[0].attr = SPR_ATTR(0, 0, 0, 1, 0);
	cspr_param[0].prio = 0;
	cspr_param[0].use_dma = true;

	uint16_t frame = 0;
	uint16_t selected_spr = 0;
	bool anim_pause = false;
	while (true)
	{
		frame++;

		md_vdp_set_bg_color(19);

		// Animate and render
		if (!anim_pause)
		{
			for (uint16_t i = 0; i < ARRAYSIZE(cspr_param); i++)
			{
				if (frame % 8 == 0) cspr_param[i].frame++;
				const uint16_t max = cspr_get_ref_count(cspr_param[i].cspr_data);
				if (cspr_param[i].frame >= max)
				cspr_param[i].frame = 0;
			}
		}

		// Selection
		if (g_md_pad_pos[0] & BTN_A)
		{
			selected_spr++;
			if (selected_spr >= ARRAYSIZE(cspr_param)) selected_spr = 0;
		}

		if (g_md_pad_pos[0] & BTN_START) anim_pause = !anim_pause;

		// Movement of selected sprite
		if (g_md_pad[0] & BTN_UP) cspr_param[selected_spr].y--;
		if (g_md_pad[0] & BTN_DOWN) cspr_param[selected_spr].y++;
		if (g_md_pad[0] & BTN_LEFT) cspr_param[selected_spr].x--;
		if (g_md_pad[0] & BTN_RIGHT) cspr_param[selected_spr].x++;
		if (g_md_pad_pos[selected_spr] & BTN_C) cspr_param[selected_spr].attr ^= SPR_ATTR(0, 1, 0, 0, 0);
		if (g_md_pad_pos[selected_spr] & BTN_B) cspr_param[selected_spr].attr ^= SPR_ATTR(0, 0, 1, 0, 0);

		md_vdp_set_bg_color(1);
		for (uint16_t i = 0; i < ARRAYSIZE(cspr_param); i++)
		{
			md_cspr_put_st(&cspr_param[i]);
		}
		md_vdp_set_bg_color(0);

		megadrive_finish();
	}

}
