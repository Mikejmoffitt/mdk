/* md-toolchain sprite support
Michael Moffitt 2018-2020 */

#ifndef MD_SPR_H
#define MD_SPR_H

#include "md/vdp.h"
#include "md/dma.h"

#define SPR_ATTR(_tile, _hf, _vf, _pal, _prio) VDP_ATTR(_tile, _hf, _vf, _pal, _prio)
#define SPR_SIZE(w, h) (((h-1) & 0x3) | (((w-1) & 0x3) << 2))

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
static inline void md_spr_init(void);

// Place a sprite using screen position coordinates.
static inline void md_spr_put(int16_t x, int16_t y, uint16_t attr, uint8_t size);

// Masks off any sprites on scanlines that span between y and the height.
static inline void md_spr_mask_line_full(int16_t y, uint8_t size);

// Masks off any sprites on scanlines that intersect two sprite positions.
static inline void md_spr_mask_line_comb(int16_t y1, uint8_t size1,
                                      int16_t y2, uint8_t size2);

// Internal Use ---------------------------------------------------------------

// Terminate the sprite list and schedule a DMA. Called by megadrive_finish().
void md_spr_finish(void);

// Static implementations =====================================================

static inline void md_spr_init(void)
{
	g_sprite_table[0].ypos = 0;
	g_sprite_count = 0;
	md_spr_finish();
}

static inline void md_spr_put(int16_t x, int16_t y, uint16_t attr, uint8_t size)
{
	if (g_sprite_count >= SPR_MAX) return;
	if (x <= -32 || x >= 320) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y + 128;
	spr->size = size;
	spr->link = ++g_sprite_count;
	spr->attr = attr;
	spr->xpos = x + 128;
}

// Masks off any later sprites within a scanline.
static inline void md_spr_mask_line_full(int16_t y, uint8_t size)
{
	if (g_sprite_count >= SPR_MAX) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y + 128;
	spr->size = size;
	spr->link = ++g_sprite_count;
	spr->xpos = 0;
}

static inline void md_spr_mask_line_overlap(int16_t y1, uint8_t size1,
                                         int16_t y2, uint8_t size2)
{
	if (g_sprite_count >= SPR_MAX - 1) return;
	SprSlot *spr = &g_sprite_table[g_sprite_count];
	spr->ypos = y1 + 128;
	spr->size = size1;
	spr->link = ++g_sprite_count;
	spr->xpos = 0;
	spr++;
	spr->ypos = y2 + 128;
	spr->size = size2;
	spr->link = ++g_sprite_count;
	spr->xpos = 1;
}

#endif  // MD_SPR_H
