// mdk sprite support
// Michael Moffitt 2018-2024
//
// MDK offers two modes of sprite management: SIMPLE and DIRECT.
//
// SIMPLE allows the use of MDK's placement functions, where linking and
// termination do not need to be considered by the programmer. Furthermore,
// composite sprite drawing functions allow for large "metasprites" to be drawn
// from several hardware sprites in one pass. Sprite list termination is
// handled automatically.
//
// DIRECT mode exposes a list of sprite data that directly corresponds to the
// data transmitted to the VDP. Management of the size and link fields must be
// done consciously by the programmer.
//
// For either mode, at the end of a frame (When megadrive_finish() is called)
// the sprite list is queued for a DMA transfer to the VDP.
//
//
// Notes on usage of the modes is below.
//
//
// SIMPLE mode interface
// ---------------------
// These routines allow for easy sprite placement without consideration for how
// sprites are ordered in the sprite table. Positions are offset so that (0, 0)
// is the top-left corner of the screen, and the sprite link value is set
// automatically so that sprites are drawn in order.
//
// Usage of SIMPLE mode:
//
// During init: This is called by megadrive_init(), but no harm in repeating it.
//     md_spr_init(SPR_MODE_SIMPLE);
//
// In game logic:
//     md_spr_put(10, 20, SPR_ATTR(0x1234, 0, 0, 0, 0), SPR_SIZE(2, 2));
//
// This places a 2x2 tile sprite at 10, 20, with tile index 0x1234, palette 0.
//
// It is also possible to invoke sprite draw commands by using a parameter
// struct, to avoid making expensive stack-based function calls. This also
// allows for re-use of previous parameters if only some fields are changed.
//
//     md_spr_put_st(&spr_param_struct);
//
// Nothing more is needed to place sprites. The call to megadrive_finish() will
// handle termination and DMA transfer of the sprite list, as well as the
// preparation of the list for the next frame.
//
// In addition to the SIMPLE drawing parameters, composite sprite functions can
// be used. Please see cspr.h for more details.
//
//
//
// DIRECT mode interface
// ---------------------
// With DIRECT mode, g_sprite_table is modified directly and transferred to the
// VDP without any automatic intervention aside from the DMA itself.
// g_sprite_table may be modified during the frame, and objects can even hold
// pointers to SprSlot entries within. There are no restrictions placed on what
// you do with the data in that list, so you could even go so far as to have
// the position fields act as object data if you were so inclined.
//
// The link field is no longer automatically managed, but the call to
// md_spr_init(SPR_MODE_DIRECT) will configure the link fields so that all
// sprites are drawn in sequence. Therefore, you may either hide a sprite by
// placing it at a negative coordinate <= -32, or update the link fields
// manually to terminate the list (point back at 0). As there is no bandwidth
// advantage to the latter, I suggest doing the former instead.
//
// Mind that in DIRECT mode, sprite coordinates are not offset for you, so a
// position of 128, 128 is in fact the top-left of the visible area.
//
// Usage of DIRECT mode:
//
// During init: This must be called as megadrive_init() defaults to SIMPLE mode.
//     md_spr_init(SPR_MODE_DIRECT);
//
// In game logic:
//     g_sprite_table[0].xpos = 10 + 128;
//     g_sprite_table[0].ypos = 20 + 128;
//     g_sprite_table[0].attr = SPR_ATTR(0x1234, 0, 0, 0, 0);
//     g_sprite_table[0].size = SPR_SIZE(2, 2);
//
// This places a 2x2 tile sprite at 10, 20, with tile index 0x1234, palette 0.
//
// Nothing more is needed at this point. megadrive_finish() will still schedule
// the DMA transfer, but it it will not touch the link fields for you like it
// does in SIMPLE mode.
//
// Optionally, g_sprite_count may be set to a value less than 80 (or 64 in H32)
// and you may terminate the sprite list prematurely, should you wish to save
// on DMA bandwidth by transferring less sprites. Otherwise, the default value
// of 80 will be adequate and you do not need to touch it.
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "md/vdp.h"
#include "md/macro.h"

// =============================================================================
// Macros
// =============================================================================
#define SPR_ATTR(tile, hf, vf, pal, prio) VDP_ATTR(tile, hf, vf, pal, prio)
#define SPR_SIZE(w, h) (((h-1) & 0x3) | (((w-1) & 0x3) << 2))

// Number of sprites in the sprite table, corresponding to the maximum supported.
#define MD_SPR_MAX 80

// =============================================================================
// Data Structures
// =============================================================================

// Operating modes for the sprite system.
typedef enum SprMode
{
	SPR_MODE_SIMPLE,  // This is the default.
	SPR_MODE_DIRECT,
} SprMode;

// Parameters for md_spr_put_st().
typedef struct SprParam
{
	int16_t x, y;  // Screen coordinates. (0, 0) is the top-left corner.
	uint16_t attr;  // See SPR_ATTR macro.
	uint8_t size;  // See SPR_SIZE macro.
} SprParam;

// Megadrive hardware sprite slot.
typedef struct SprSlot
{
	uint16_t ypos;
	union
	{
		struct
		{
			uint8_t size;
			uint8_t link;  // Range is 0x00-0x7F.
		};
		uint16_t sizelink;
	};
	uint16_t attr;
	uint16_t xpos;
} SprSlot;

// =============================================================================
// Data
// =============================================================================

// The sprite table cache. This is transferred to the VDP via DMA.
// It is used by both SIMPLE and DIRECT mode, but user modification is only
// recommended when using DIRECT mode.
extern SprSlot g_sprite_table[MD_SPR_MAX];
extern SprSlot *g_sprite_next;

// In SIMPLE mode, this is used by the md_spr_put() functions as a counter.
// In DIRECT mode, it controls how many sprite slots are transferred via DMA.
extern uint16_t g_sprite_count;

// =============================================================================
// Interface
// =============================================================================

// Clears sprites and initializes sprite cache with link data.
// This may be called multiple times in order to change modes or clear sprites.
// In both SIMPLE and DIRECT modes, the link field is populated in a linear
// order, with sprite 0 pointing to sprite 1, sprite 1 pointing to sprite 2,
// and so on and so forth.
// In DIRECT mode, g_sprite_count is set to the maximum sprite number for the
// current video mode (H40 --> 80 sprites; H32 --> 64 sprites).
void md_spr_init(SprMode mode);

// Place a sprite using screen position coordinates.
void md_spr_put(int16_t x, int16_t y, uint16_t attr, uint16_t size);

// (Parameter struct reference version).
void md_spr_put_st(const SprParam *s);

// Same as put_st, except it is a tiny bit faster by virtue of the following:
// * Sprites are not automatically offset by 128px
// * Bounds checks are not performed on sprite coordinates
void md_spr_put_st_fast(const SprParam *s);

// Masks off any sprites on scanlines that span between y and the height.
void md_spr_mask_line_full(int16_t y, uint8_t size);

// Masks off any sprites on scanlines that intersect two sprite positions.
void md_spr_mask_line_comb(int16_t y1, uint8_t size1,
                           int16_t y2, uint8_t size2);

// =============================================================================
// Internal Functions
// =============================================================================

// Prepares the sprite cache for a frame. Called by megadrive_finish().
void md_spr_start(void);

// Terminate the sprite list and schedule a DMA. Called by megadrive_finish().
void md_spr_finish(void);

#ifdef __cplusplus
}
#endif  // __cplusplus
