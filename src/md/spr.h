/* md-toolchain sprite support
Michael Moffitt */

#ifndef SPR_H
#define SPR_H

#include "md/vdp.h"
#include "md/dma.h"

#define SPR_ATTR(_tile, _hf, _vf, _pal, _prio) VDP_ATTR(_tile, _hf, _vf, _pal, _prio)
#define SPR_SIZE(w, h) (((h-1) & 0x3) | (((w-1) & 0x3) << 2))

#define SPR_Y_HIDDEN -128
#define SPR_X_PRIO_CANCEL -128

#define SPR_MAX 80

typedef struct SprSlot
{
	uint16_t ypos;
	uint8_t size;
	uint8_t link;
	uint16_t attr;
	uint16_t xpos;
} SprSlot;

extern SprSlot g_sprite_table[SPR_MAX];
extern SprSlot *g_sprite_next; // Points to the next open sprite slot.
extern uint8_t g_sprite_count;

// Clears sprites and initialize g_sprite_next.
static inline void spr_init(void);

// Place a sprite. Tile index is VRAM position / 32.
static inline void spr_put(int16_t xpos, int16_t ypos, uint16_t attr, uint8_t size);

// Terminate the sprite list and schedule a DMA. Resets g_sprite_next.
void spr_finish(void);

static inline void spr_init(void)
{
	g_sprite_table[0].ypos = SPR_Y_HIDDEN;
	g_sprite_count = 0;
	spr_finish();
}

// Place a sprite.
static inline void spr_put(int16_t xpos, int16_t ypos, uint16_t attr, uint8_t size)
{
	if (g_sprite_count >= SPR_MAX)
	{
		return;
	}
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->link = g_sprite_count + 1;
	spr->xpos = xpos + 128;
	spr->ypos = ypos + 128;
	spr->attr = attr;
	spr->size = size;
	g_sprite_count++;
}
#endif // SPR_H
