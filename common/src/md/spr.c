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
		g_sprite_table[0].ypos = SPR_Y_HIDDEN;
		g_sprite_count = 1;
	}

	dma_q_transfer_vram(vdp_get_sprite_base(), (void *)g_sprite_table,
	                    sizeof(SprSlot) * g_sprite_count / 2, 2);

	g_sprite_count = 0;
	g_sprite_next = &g_sprite_table[0];
}


