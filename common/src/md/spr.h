/* md-toolchain sprite support
Michael Moffitt 2018-2020 */

#ifndef MD_SPR_H
#define MD_SPR_H

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

// Terminate the sprite list and schedule a DMA. Resets g_sprite_next.
void spr_finish(void);

static inline void spr_init(void)
{
	g_sprite_table[0].ypos = SPR_Y_HIDDEN;
	g_sprite_count = 0;
	spr_finish();
}

// Place a sprite. Tile index is VRAM position / 32.
static inline void spr_put(int16_t x, int16_t y, uint16_t attr, uint8_t size)
{
	if (g_sprite_count >= SPR_MAX) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y + 128;
	spr->size = size;
	spr->link = ++g_sprite_count;
	spr->attr = attr;
	spr->xpos = x + 128;
}
#endif  // MD_SPR_H
