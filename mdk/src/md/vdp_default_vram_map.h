#ifndef MD_VDP_DEFAULT_VRAM_MAP_H
#define MD_VDP_DEFAULT_VRAM_MAP_H

// -----------------------------------------------------------------------------
// Default VRAM layout - Optimized for 64x32 cell planes.
// These are used during init, but you are not obligated to stick to them.
// -----------------------------------------------------------------------------

// 0000-BFFF (1536 tiles) free
#define VRAM_SCRA_BASE_DEFAULT  0xC000
#define VRAM_SCRW_BASE_DEFAULT  0xD000
#define VRAM_SCRB_BASE_DEFAULT  0xE000
// F000-F7FF (64 tiles) free
#define VRAM_HSCR_BASE_DEFAULT  0xF800
#define VRAM_SPR_BASE_DEFAULT   0xFC00

#endif  // MD_VDP_DEFAULT_VRAM_MAP_H
