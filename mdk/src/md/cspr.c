#include "md/cspr.h"
#include "md/spr.h"

#include "md/dma.h"
#include "md/pal.h"

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

#define MD_CSPR_NAME 0x00
#define MD_CSPR_PAL 0x10
#define MD_CSPR_REF_COUNT 0x30
#define MD_CSPR_TILE_DATA_OFFSET 0x36
#define MD_CSPR_TILE_COUNT 0x3A

// Gets the quantity of animation frames defined in cspr_data.
uint16_t md_cspr_get_frame_count(const void *cspr_data)
{
	const uint8_t *cspr = (const uint8_t *)cspr_data;
	const uint16_t ref_count = *(uint16_t *)(&cspr[MD_CSPR_REF_COUNT]);
	return ref_count;
}

// Loads all tile data from cspr_data to VRAM, for non-DMA usage.
void md_cspr_upload_tiles(const void *cspr_data, uint16_t vram_addr)
{
	const uint8_t *cspr = (const uint8_t *)cspr_data;
	const uint32_t tile_data_offset = *(uint32_t *)(&cspr[MD_CSPR_TILE_DATA_OFFSET]);
	const uint8_t *tile_data = cspr + tile_data_offset;
	const uint16_t tile_count = *(uint16_t *)(&cspr[MD_CSPR_TILE_COUNT]);
	if (tile_count == 0) return;
	md_dma_transfer_vram(vram_addr, tile_data, tile_count * 16, 2);
}

// Loads the palette from cspr_data to a palette line.
void md_cspr_upload_pal(const void *cspr_data, uint16_t pal_line)
{
	const uint8_t *cspr = (const uint8_t *)cspr_data;
	const uint16_t *pal_data = (const uint16_t *)(&cspr[MD_CSPR_PAL]);
	md_pal_upload(pal_line * 16, pal_data, 16);
}

// Returns a pointer to a C-string of the name of the composite sprite.
const char *md_cspr_get_name(const void *cspr_data)
{
	const uint8_t *cspr = (const uint8_t *)cspr_data;
	return (const char *)(&cspr[MD_CSPR_NAME]);
}


