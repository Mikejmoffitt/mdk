#pragma once
#include	"md/vdp_regs.h"

/* Macro for VDP address calculation (https://plutiedef.com/writing-video) */
	.macro	md_set_xram_addr	addr, cmd
	move.l	#(((\addr)&0x3FFF)<<16) | (((\addr)&0xC000)>>14) | (\cmd), (VDP_LOC_BASE+VDP_OFFS_CTRL)
	.endm

	.macro	md_set_vram_addr	addr
	md_set_xram_addr	\addr, VDP_CTRL_VRAM_WRITE
	.endm

	.macro	md_set_cram_addr	addr
	md_set_xram_addr	\addr, VDP_CTRL_CRAM_WRITE
	.endm

	.macro	md_set_vsram_addr	addr
	md_set_xram_addr	\addr, VDP_CTRL_VSRAM_WRITE
	.endm

/* Macro to wait for DMA completion. */
	.macro	dma_wait
9:	btst	#1, VDP_LOC_BASE+VDP_OFFS_DATA+1
	bne	9b
	.endm
