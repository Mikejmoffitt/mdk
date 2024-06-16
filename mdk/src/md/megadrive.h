// mdk Top-level Sega Megadrive support routines and structures
// 2018-2024 Michael Moffitt
// ===========================================================================
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include "md/macro.h"       // Helpful macros.
#include "md/vdp.h"         // VDP control: all things graphics and some more
#include "md/sys.h"         // System control: interrupts, sub-CPU control
#include "md/io.h"          // Controller port I/O
#include "md/ioc.h"         // System C/C2 I/O
#include "md/spr.h"         // Sprite support
#include "md/cspr.h"        // Composite sprite functions
#include "md/dma.h"         // DMA control and scheduling
#include "md/pal.h"         // Palette read/write via ports or DMA
#include "md/opn.h"         // YM2610 FM sound chip
#include "md/psg.h"         // SN76489-compatible PSG sound chip
#include "md/sram.h"        // Support for battery-backed SRAM
#include "md/irq.h"         // Interrupt handler registration
#include "md/sysc_vctrl.h"  // System C/C2 Video Control

// Run after completing the logic in one game tick loop.
static inline void megadrive_finish(void)
{
	md_spr_finish();
	md_pal_poll();
	md_vdp_wait_vblank();

	// C2-specific screen blanking.
#ifdef MDK_TARGET_C2
	md_sysc_vctrl_set_blank(1);
#endif  // MDK_TARGET_C2

#ifndef MDK_TARGET_C2
	md_io_poll();
#else
	md_ioc_poll();
#endif  // MDK_TARGET_C2
	md_dma_process();

	md_spr_start();

	// C2-specific screen blanking.
#ifdef MDK_TARGET_C2
	md_sysc_vctrl_set_blank(0);
#endif  // MDK_TARGET_C2
}

// Internal use ---------------------------------------------------------------

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

#ifdef __cplusplus
}
#endif  // __cplusplus
