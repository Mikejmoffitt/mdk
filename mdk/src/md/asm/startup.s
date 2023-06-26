/* mdk header, interrupts, and C runtime startup
Michael Moffitt 2018-2022 */
	.include	"md/asm/vdp_defs.inc"
	.include	"md/asm/z80_defs.inc"

	.section	.text.keepboot

	.extern	_etext
	.extern	_stext
	.extern	_edata
	.extern	_sdata
	.extern	main

	.extern	_v_irq1
	.extern	_v_irq2
	.extern	_v_irq3
	.extern	_v_irq4
	.extern	_v_irq5
	.extern	_v_irq6
	.extern	_v_irq7

	.global	_v_table
	.global	_start
	.org	0x00000000
_v_table:
	/* initial sp */
	.long	0x1000000
	/* reset vector; entry point */
	.long	start
	/* bus error */
	.long	_v_bus_error
	/* address error */
	.long	_v_address_error
	/* illegal instruction */
	.long	_v_illegal_instruction
	/* divide by zero error */
	.long	_v_div_zero
	/* CHK out of bounds */
	.long	_v_chk
	/* TRAPV with overflow set */
	.long	_v_trapv
	/* Privelege violation */
	.long	_v_privelege
	/* Trace */
	.long	_v_trace
	/* A-line */
	.long	_v_aline_emu
	/* F-line */
	.long	_v_fline_emu
	/* reserved */
	.long	_v_reserved
	/* coproc violation? */
	.long	_v_coproc_violation
	/* format error */
	.long	_v_format
	/* uninitialized interrupt vector */
	.long	_v_uninit
	/* reserved on 68000 */
	.long	_v_reserved
	.long	_v_reserved
	.long	_v_reserved
	.long	_v_reserved
	.long	_v_reserved
	/* reserved; used on Neo Geo CD */
	.long	_v_reserved
	.long	_v_reserved
	.long	_v_reserved
	/* spurious interrupt */
	.long	_v_spurious
	/* L1-7 interrupts interrupt */
	.long	_v_irq1
	.long	_v_irq2
	.long	_v_irq3
	.long	_v_irq4
	.long	_v_irq5
	.long	_v_irq6
	.long	_v_irq7
	/* Trap instructions */
	.long	_v_trap0x0
	.long	_v_trap0x1
	.long	_v_trap0x2
	.long	_v_trap0x3
	.long	_v_trap0x4
	.long	_v_trap0x5
	.long	_v_trap0x6
	.long	_v_trap0x7
	.long	_v_trap0x8
	.long	_v_trap0x9
	.long	_v_trap0xa
	.long	_v_trap0xb
	.long	_v_trap0xc
	.long	_v_trap0xd
	.long	_v_trap0xe
	.long	_v_trap0xf
	/* Unimplemented (FPU) */
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	/* Unimplemented (MMU) */
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	/* Unimplemented (reserved) */
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp
	.long	_v_unimp

.include	"header.inc"

_start:
start:
	move.w	#0x2700, sr  /* Ints off */
	move.l	(0x01000000).l, sp

	/* Halt Z80 */
	move.w	#0x0100, (Z80_BUS_LOC).l

	/* Initialize TMSS, if relevant */
.ifndef MDK_SYSTEM_C2
	move.b	0xA10001, d0
	andi.b	#0x0F, d0
	beq	2f
	move.l	#0x53454741, 0xA14000
2:
.endif
	/* Shut up PSG - On my VA0 units it defaults to a loud noise. */
	lea	0xC00011, a0
	move.b	#0x9F, d0  /* Channel 0 att 15 */
	move.b	#0x20, d1  /* Channel offset */
	move.b	d0, (a0)
	add.b	d1, d0
	move.b	d0, (a0)
	add.b	d1, d0
	move.b	d0, (a0)
	add.b	d1, d0
	move.b	d0, (a0)

	lea	softreset, a6
	bra	startup_vdp_init

softreset:
	/* Prepare controller read */
	move.l	#0x00400040, (0xA10008).l  /* Set TH pin as an output */
	move.b	#0x00, (0xA10003).l  /* TH low */

	/* Turn off display output */
	move.w	#VDP_REGST_MODESET2 | 0x04, (VDPPORT_CTRL)

	/* Check for start button */
	btst	#5, (0xA10003).l  /* Check start button */
	bne	md_crt0_begin

	lea	wram_test_begin, a6
	jmp	md_error_startup_wram_check_display

wram_test_begin:
	/* Test RAM if Start is held */
	move.l	#0x5555, d0
	lea	wram_test_pass1, a6
	bra	startup_wram_test_sub
wram_test_pass1:
	move.l	#0xAAAA, d0
	lea	wram_test_pass2, a6
	bra	startup_wram_test_sub
wram_test_pass2:
	lea	startup_forever, a6
	jmp	md_error_startup_wram_ok_display
	bra	md_crt0_begin

# d0 = test word
# a6 = return
startup_wram_test_sub:
	move.l	#0x00FF0000, a4
	move.w	#(0x8000/2) - 1, d7
wram_test_loop:
	move.w	d0, (a4)
	cmp.w	(a4)+, d0
	bne	wram_test_failed
	dbra	d7, wram_test_loop
	jmp	(a6)
wram_test_failed:
	lea	startup_forever, a6
	jmp	md_error_startup_wram_ng_display

startup_forever:
	move.b	#0x00, (0xA10003).l  /* TH low */
	btst	#5, (0xA10003).l  /* Check start button */
	bne	startup_forever

startup_forever_wait_start_release:
	btst	#5, (0xA10003).l  /* Check start button */
	beq	startup_forever_wait_start_release
	bra	start


#
# Very basic VDP init and font install for the sake of diagnostics.
# Does NOT clobber d0.
#
# a6 = return
#
startup_vdp_init:
	lea	VDPPORT_DATA, a4
	lea	VDPPORT_CTRL, a5

	/* Basic VDP Init */
	lea	vdp_init_reg_tbl, a0
reg_init_copy_top:
	move.w	(a0)+, d1
	bpl	reg_init_done
	move.w	d1, (a5)
	bra	reg_init_copy_top

reg_init_done:
	/* Clear VRAM */
	md_set_vram_addr 0x0000
	moveq	#0, d1
	move.w	#(0x10000/4) - 1, d7
vdp_mem_clear_top:
	move.l	d1, (a4)
	dbf	d7, vdp_mem_clear_top

	/* Write palette for font */
	md_set_cram_addr 0x0000
	moveq	#(128 / (4 * 4)) - 1, d7
	lea	md_error_palette, a0
pal_copy_top:
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	dbf	d7, pal_copy_top

	/* Copy font into VRAM */
	md_set_vram_addr 0x0000
	lea	md_error_font, a0
	move.w	#((12 * 16 * 32) / (4 * 4)) - 1, d7
vram_copy_top:
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	move.l	(a0)+, (a4)
	dbf	d7, vram_copy_top

	/* Return handled with a6 */
	jmp	(a6)

vdp_init_reg_tbl:
	dc.w	VDP_REGST_MODESET1 | 0x04
	dc.w	VDP_REGST_MODESET2 | 0x04
	dc.w	VDP_REGST_MODESET3 | 0x00
.ifdef MDK_SYSTEM_C2
	dc.w	VDP_REGST_MODESET4 | 0x51
.else
	dc.w	VDP_REGST_MODESET4 | 0x81
.endif	
	dc.w	VDP_REGST_SCRABASE | (0x2000 >> 10)
	dc.w	VDP_REGST_SCRBBASE | (0x4000 >> 13)
	dc.w	VDP_REGST_SPRBASE  | 0x78
	dc.w	VDP_REGST_SCRWBASE | (0x3000 >> 10)
	dc.w	VDP_REGST_HSCRBASE | 0x3D

	dc.w	VDP_REGST_PLANESIZE | 0x01
	dc.w	VDP_REGST_WINHORI | 0x1F
	dc.w	VDP_REGST_WINVERT | 0x1F
	dc.w	VDP_REGST_AUTOINC | 0x02
	dc.w	VDP_REGST_BGCOL | 0x00
	dc.w	VDP_REGST_HINTC | 0xFF
	dc.w	0  /* end marker */

#
# Graphics
#

md_error_palette:
#
	dc.w	0x0000
	dc.w	0x0EEE
	dc.w	0x0EEC
	dc.w	0x0EEA
	dc.w	0x0EE8
	dc.w	0x0EE6
	dc.w	0x0EE4
	dc.w	0x0EE2
	dc.w	0x0EE0
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0222
	dc.w	0x0000
#
	dc.w	0x0000
	dc.w	0x0EE0
	dc.w	0x0CE2
	dc.w	0x0AE4
	dc.w	0x08E6
	dc.w	0x06E8
	dc.w	0x04EA
	dc.w	0x02EC
	dc.w	0x00EE
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0222
	dc.w	0x0000
#
	dc.w	0x0000
	dc.w	0x00EE
	dc.w	0x00CE
	dc.w	0x00AE
	dc.w	0x008E
	dc.w	0x006E
	dc.w	0x004E
	dc.w	0x002E
	dc.w	0x000E
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0222
	dc.w	0x0000
#
	dc.w	0x0000
	dc.w	0x0E2E
	dc.w	0x0C2E
	dc.w	0x0A2E
	dc.w	0x082E
	dc.w	0x062E
	dc.w	0x042E
	dc.w	0x022E
	dc.w	0x002E
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0000
	dc.w	0x0222
	dc.w	0x0000

md_error_font:
	.incbin	"md/asm/font.bin"

# TODO: Checksum


#
# Vectors
#

_v_bus_error:
	moveq	#0, d0
	bra	startup_error_display
_v_address_error:
	moveq	#1, d0
	bra	startup_error_display
_v_illegal_instruction:
	moveq	#2, d0
	bra	startup_error_display
_v_div_zero:
	moveq	#3, d0
	bra	startup_error_display
_v_chk:
	moveq	#4, d0
	bra	startup_error_display
_v_trapv:
	moveq	#5, d0
	bra	startup_error_display
_v_privelege:
	moveq	#6, d0
	bra	startup_error_display
_v_trace:
	moveq	#7, d0
	bra	startup_error_display
_v_unused_irq:
	moveq	#8, d0
	bra	startup_error_display
_v_aline_emu:
	moveq	#9, d0
	bra	startup_error_display
_v_fline_emu:
	moveq	#10, d0
	bra	startup_error_display
_v_reserved:
	moveq	#11, d0
	bra	startup_error_display
_v_coproc_violation:
	moveq	#12, d0
	bra	startup_error_display
_v_format:
	moveq	#13, d0
	bra	startup_error_display
_v_uninit:
	moveq	#14, d0
	bra	startup_error_display
_v_spurious:
	moveq	#15, d0
	bra	startup_error_display
_v_trap0x0:
	moveq	#16, d0
	bra	startup_error_display
_v_trap0x1:
	moveq	#17, d0
	bra	startup_error_display
_v_trap0x2:
	moveq	#18, d0
	bra	startup_error_display
_v_trap0x3:
	moveq	#19, d0
	bra	startup_error_display
_v_trap0x4:
	moveq	#20, d0
	bra	startup_error_display
_v_trap0x5:
	moveq	#21, d0
	bra	startup_error_display
_v_trap0x6:
	moveq	#22, d0
	bra	startup_error_display
_v_trap0x7:
	moveq	#23, d0
	bra	startup_error_display
_v_trap0x8:
	moveq	#24, d0
	bra	startup_error_display
_v_trap0x9:
	moveq	#25, d0
	bra	startup_error_display
_v_trap0xa:
	moveq	#26, d0
	bra	startup_error_display
_v_trap0xb:
	moveq	#27, d0
	bra	startup_error_display
_v_trap0xc:
	moveq	#28, d0
	bra	startup_error_display
_v_trap0xd:
	moveq	#29, d0
	bra	startup_error_display
_v_trap0xe:
	moveq	#30, d0
	bra	startup_error_display
_v_trap0xf:
	moveq	#31, d0
	bra	startup_error_display
_v_unimp:
	moveq	#32, d0
	bra	startup_error_display

startup_error_display:
	lea	startup_error_display_init_post, a6
	bra	startup_vdp_init
startup_error_display_init_post:
	jsr	md_error_exception_display
	lea	startup_forever, a6
	jmp	md_error_startup_start_display

#
# Other routines kept explicitly in low ROM
#

.include	"md/asm/crt0.inc"
.include	"md/asm/dma_process.inc"
.include	"md/asm/sram.inc"
