// md-toolchain Top-level Sega Megadrive support routines and structures
// 2018-2020 Michael Moffitt
// ===========================================================================


#ifndef MD_MEGADRIVE_H
#define MD_MEGADRIVE_H

#include "md/vdp.h"   // VDP control: all things graphics and some more
#include "md/sys.h"   // System control: interrupts, sub-CPU control
#include "md/io.h"    // Controller port I/O
#include "md/spr.h"   // Sprite support
#include "md/dma.h"   // DMA control and scheduling
#include "md/pal.h"   // Palette read/write via ports or DMA
#include "md/opn.h"   // YM2610 FM sound chip
#include "md/psg.h"   // SN76489-compatible PSG sound chip
#include "md/sram.h"  // Support for battery-backed SRAM

// (optional) default startup.
// * Initializes VDP with VDP defaults (see vdp.c):
//     * Vertical blank interrupt enabled
//     * 320 x 224 (40x28 cell) video mode
//     * Clear VRAM
//     * Display enabled
//     * DMA enabled
//     * HV counter enabled
//     * VRAM bases:  SCRL A  SCRL B  WINDOW  SPRITES  H-SCROLL
//                    0xB000  0xD000  0xF000  0xFC00   0xFA00
//     * Enables Plane Scrolling
//     * Sets Palette 0 Color 0 for the backdrop / border
//     * VRAM auto-inc set to 2
//     * 64 x 32 cell plane size
// * Clears sprites
// * Configures IO ports for gamepad reads
// * Enables interrupts
void megadrive_init(void);

// Run after completing the logic in one game tick loop.
static inline void megadrive_finish(void)
{
	spr_finish();
	vdp_wait_vblank();
	spr_dma(/*now=*/1);
	io_poll();
	dma_q_process();
}

#endif // MD_MEGADRIVE_H
