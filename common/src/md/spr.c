/* md-toolchain sprite support
Michael Moffitt */
#include "md/spr.h"

SprSlot g_sprite_table[SPR_MAX];
SprSlot *g_sprite_next; // Points to the next open sprite slot.
uint8_t g_sprite_count;

void spr_finish(void)
{
	if (g_sprite_count > 0)
	{
		g_sprite_table[g_sprite_count - 1].link = 0;
	}
	else
	{
		g_sprite_table[0].link = 0;
		g_sprite_table[0].ypos = 0;
		g_sprite_count = 1;
	}

	const uint8_t sprite_count_for_dma = g_sprite_count;

	g_sprite_count = 0;
	g_sprite_next = &g_sprite_table[0];

	dma_q_transfer_spr_vram(vdp_get_sprite_base(), (void *)g_sprite_table,
	                        sizeof(SprSlot) * sprite_count_for_dma / 2, 2);
}
