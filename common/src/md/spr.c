/* md-toolchain sprite support
Michael Moffitt */
#include "md/spr.h"

SprSlot g_sprite_table[SPR_MAX];
SprSlot *g_sprite_next; // Points to the next open sprite slot.
uint8_t g_sprite_count;

static uint8_t s_sprite_count_for_dma;

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

	s_sprite_count_for_dma = g_sprite_count;

	g_sprite_count = 0;
	g_sprite_next = &g_sprite_table[0];
}


// Run or schedule a DMA for the sprite list. Call after spr_finish().
// This function is separate so that it is easier to force the sprite list
// update to occur before other transfers.
void spr_dma(int16_t now)
{
	if (now)
	{
		dma_set_stride(2);
		dma_transfer_vram(vdp_get_sprite_base(), (void *)g_sprite_table,
		                  sizeof(SprSlot) * s_sprite_count_for_dma / 2);
	}
	else
	{
		dma_q_transfer_vram(vdp_get_sprite_base(), (void *)g_sprite_table,
		                    sizeof(SprSlot) * s_sprite_count_for_dma / 2, 2);
	}
}
