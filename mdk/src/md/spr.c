/* mdk sprite support
Michael Moffitt */
#include "md/spr.h"
#include "md/dma.h"

#define SPR_STATIC_OFFS 128

SprSlot g_sprite_table[MD_SPR_MAX];
SprSlot *g_sprite_next;
uint16_t g_sprite_count;

static SprMode s_mode;

void md_spr_init(SprMode mode)
{
	s_mode = mode;
	// The sprite table has its link values installed ahead of time.
	for (uint16_t i = 0; i < MDK_ARRAYSIZE(g_sprite_table); i++)
	{
		g_sprite_table[i].link = i + 1;
		g_sprite_table[i].xpos = 2;  // Avoid triggering line mask.
	}

	g_sprite_next = &g_sprite_table[0];

	switch (s_mode)
	{
		case SPR_MODE_SIMPLE:
			md_spr_start();
			md_spr_finish();
			break;
		case SPR_MODE_DIRECT:
			g_sprite_count = (md_vdp_get_hmode() == VDP_HMODE_H40) ? 80 : 64;
			break;
	}
}

void md_spr_start(void)
{
	if (s_mode != SPR_MODE_SIMPLE) return;
	// Restoration of link field.
	g_sprite_table[0].link = 1;
	if (g_sprite_count > 0)
	{
		g_sprite_table[g_sprite_count - 1].link = g_sprite_count;
		g_sprite_count = 0;
	}
	g_sprite_next = &g_sprite_table[0];
}

void md_spr_finish(void)
{
	uint8_t transfer_count = g_sprite_count;
	if (s_mode == SPR_MODE_SIMPLE)
	{
		if (g_sprite_count == 0)
		{
			// Sets sprite 0 to be invisible, and point at itself. This is used
			// to terminate the list in the absence of any sprite placements.
			g_sprite_table[0].link = 0;
			g_sprite_table[0].ypos = 0;
			g_sprite_table[0].size = 0;
			transfer_count = 1;
		}
		else
		{
			// The last sprite in the table is terminated with a link back to zero.
			g_sprite_table[g_sprite_count - 1].link = 0;
		}
	}

	// Schedule a transfer for the sprite table.
	md_dma_transfer_spr_vram(md_vdp_get_sprite_base(), (void *)g_sprite_table,
	                         sizeof(SprSlot) * transfer_count / 2, 2);
	g_sprite_next = &g_sprite_table[0];
}

void md_spr_mask_line_full(int16_t y, uint8_t size)
{
	if (g_sprite_count >= MDK_ARRAYSIZE(g_sprite_table)) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y + SPR_STATIC_OFFS;
	spr->size = size;
	spr->xpos = 0;
	spr->attr = 0;
	spr->link = 7;
	g_sprite_count++;
	g_sprite_next++;
}

void md_spr_mask_line_overlap(int16_t y1, uint8_t size1,
                              int16_t y2, uint8_t size2)
{
	if (g_sprite_count >= MDK_ARRAYSIZE(g_sprite_table) - 1) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y1 + SPR_STATIC_OFFS;
	spr->size = size1;
	spr->xpos = 0;
	spr->attr = 0;
	spr->link = 7;
	spr++;
	spr->ypos = y2 + SPR_STATIC_OFFS;
	spr->size = size2;
	spr->xpos = 1;
	spr->attr = 0;
	spr->link = 7;
	g_sprite_count += 2;
	g_sprite_next++;
	g_sprite_next++;
}

#undef SPR_STATIC_OFFS
