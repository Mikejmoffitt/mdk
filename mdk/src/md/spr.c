/* mdk sprite support
Michael Moffitt */
#include "md/spr.h"
#include "md/dma.h"

SprSlot g_sprite_table[SPR_MAX];
uint8_t g_sprite_count;

void md_spr_init(void)
{
	// The sprite table has its link values installed ahead of time.
	for (uint16_t i = 1; i < ARRAYSIZE(g_sprite_table); i++)
	{
		g_sprite_table[i].link = i + 1;
	}

	md_spr_start();
	md_spr_finish();
}


void md_spr_start(void)
{
	// TODO: This case shouldn't ever be hit, so consider removing this.
	if (g_sprite_count == 0)
	{
		g_sprite_table[0].link = 1;
		return;
	}

	// Put back the link value that was set to zero in the last frame.
	g_sprite_table[g_sprite_count - 1].link = g_sprite_count;

	g_sprite_count = 0;
}

void md_spr_finish(void)
{
	if (g_sprite_count == 0)
	{
		// Sets sprite 0 to be invisible, and point at itself. This is used
		// to terminate the list in the absence of any sprite placements.
		g_sprite_table[0].link = 0;
		g_sprite_table[0].ypos = 0;
		g_sprite_count = 1;
	}
	else
	{
		// The last sprite in the table is terminated with a link back to zero.
		g_sprite_table[g_sprite_count - 1].link = 0;
	}

	// Schedule a transfer for the sprite table.
	md_dma_transfer_spr_vram(md_vdp_get_sprite_base(), (void *)g_sprite_table,
	                         sizeof(SprSlot) * g_sprite_count / 2, 2);
}
