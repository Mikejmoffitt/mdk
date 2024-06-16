// mdk composite sprite support
// 2023-2024 Michael Moffitt
// These functions may be used in SIMPLE mode.
#pragma once

#include <stdint.h>

// Struct for composite sprite drawing parameters.
typedef struct CSprParam
{
	const void *cspr_data;  // Composite sprite binary data struct
	uint16_t vram_base;     // Base VRAM address used for tile data.
	uint16_t frame;         // Frame within cspr struct to display.
	int16_t x, y;           // Screen coordinates.
	uint16_t attr;          // Use SPR_ATTR macro with tile parameter set to 0.
	uint16_t use_dma;
} CSprParam;

// Draws a composite sprite using screen position coordinates.
// `attr` should only have flip flags and palette set.
// You may use the SPR_ATTR macro with the tile set to 0.
void md_cspr_put_st(const CSprParam *s);

// "Fast" version with various safety checks removed for speed. Use this
// for faster drawing of objects that you are already culling when off-screen
// and will absolutely not be accessing animation frames outside what the data
// offers.
//
// Checks removed:
// * Screen boundary checks
// * Frame bound checks
void md_cspr_put_st_fast(const CSprParam *s);

// Loads all tile data from cspr_data to VRAM, for non-DMA usage.
void md_cspr_upload_tiles(const void *cspr_data, uint16_t vram_addr);

// Loads the palette from cspr_data to a palette line.
void md_cspr_upload_pal(const void *cspr_data, uint16_t pal_line);

// Gets the quantity of animation frames defined in cspr_data.
uint16_t md_cspr_get_frame_count(const void *cspr_data);

// Returns a pointer to a C-string of the name of the composite sprite.
const char *md_cspr_get_name(const void *cspr_data);
