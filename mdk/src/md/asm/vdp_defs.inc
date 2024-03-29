.extern	g_md_vdp_regs  /* uint8_t [0x18] */
.extern	g_md_vdp_debug_regs  /* uint16_t [0x10] */

# VDP MMIO
.set	VDP_LOC_BASE, 0xC00000
.set	VDPPORT_DATA, VDP_LOC_BASE
.set	VDPPORT_CTRL, VDP_LOC_BASE + 4
.set	VDPPORT_HVCOUNT, VDP_LOC_BASE + 8
.set	VDPPORT_DBG_SEL, VDP_LOC_BASE + 0x18
.set	VDPPORT_DBG_DATA, VDP_LOC_BASE + 0x1C

# VDP command constants
.set	VRAM_ADDR_CMD,  0x40000000
.set	CRAM_ADDR_CMD,  0xC0000000
.set	VSRAM_ADDR_CMD, 0x40000010

# OR these with an 8-bit value and write to the control port.
.set	VDP_REGST_MODESET1,  0x8000
.set	VDP_REGST_MODESET2,  0x8100
.set	VDP_REGST_SCRABASE,  0x8200
.set	VDP_REGST_SCRWBASE,  0x8300
.set	VDP_REGST_SCRBBASE,  0x8400
.set	VDP_REGST_SPRBASE,   0x8500
.set	VDP_REGST_128_SPCGA, 0x8600
.set	VDP_REGST_BGCOL,     0x8700
.set	VDP_REGST_UNUSED1,   0x8800
.set	VDP_REGST_UNUSED2,   0x8900
.set	VDP_REGST_HINTC,     0x8A00
.set	VDP_REGST_MODESET3,  0x8B00
.set	VDP_REGST_MODESET4,  0x8C00
.set	VDP_REGST_HSCRBASE,  0x8D00
.set	VDP_REGST_128_BPCGA, 0x8E00
.set	VDP_REGST_AUTOINC,   0x8F00
.set	VDP_REGST_PLANESIZE, 0x9000
.set	VDP_REGST_WINHORI,   0x9100
.set	VDP_REGST_WINVERT,   0x9200
.set	VDP_REGST_DMALEN1,   0x9300
.set	VDP_REGST_DMALEN2,   0x9400
.set	VDP_REGST_DMASRC1,   0x9500
.set	VDP_REGST_DMASRC2,   0x9600
.set	VDP_REGST_DMASRC3,   0x9700

# Indices into the VDP register array.
.set	VDP_MODESET1,  0x00
.set	VDP_MODESET2,  0x01
.set	VDP_SCRABASE,  0x02
.set	VDP_SCRWBASE,  0x03
.set	VDP_SCRBBASE,  0x04
.set	VDP_SPRBASE,   0x05
.set	VDP_128_SPCGA, 0x06
.set	VDP_BGCOL,     0x07
.set	VDP_UNUSED1,   0x08
.set	VDP_UNUSED2,   0x09
.set	VDP_HINTC,     0x0A
.set	VDP_MODESET3,  0x0B
.set	VDP_MODESET4,  0x0C
.set	VDP_HSCRBASE,  0x0D
.set	VDP_128_BPCGA, 0x0E
.set	VDP_AUTOINC,   0x0F
.set	VDP_PLANESIZE, 0x10
.set	VDP_WINHORI,   0x11
.set	VDP_WINVERT,   0x12
.set	VDP_DMALEN1,   0x13
.set	VDP_DMALEN2,   0x14
.set	VDP_DMASRC1,   0x15
.set	VDP_DMASRC2,   0x16
.set	VDP_DMASRC3,   0x17

.set	VDP_DBG_LAYER, 0x0000
.set	VDP_DBG_CLKST, 0x0100

# Macro to wait for DMA completion.
	.macro	dma_wait
9:	btst	#1, VDPPORT_DATA+1
	bne	9b
	.endm

# Macro for VDP address calculation (https://plutiedef.com/writing-video)

	.macro	md_set_xram_addr	addr, cmd
	move.l	#(((\addr)&0x3FFF)<<16) | (((\addr)&0xC000)>>14) | (\cmd), (VDPPORT_CTRL)
	.endm

	.macro	md_set_vram_addr	addr
	md_set_xram_addr	\addr, VRAM_ADDR_CMD
	.endm

	.macro	md_set_cram_addr	addr
	md_set_xram_addr	\addr, CRAM_ADDR_CMD
	.endm

	.macro	md_set_vsram_addr	addr
	md_set_xram_addr	\addr, VSRAM_ADDR_CMD
	.endm
