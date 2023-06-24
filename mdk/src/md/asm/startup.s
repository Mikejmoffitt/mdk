/* mdk header, interrupts, and C runtime startup
Michael Moffitt 2018-2022 */

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
	move.l	(0x000000).l, sp
	/* disable ints */
	move.w	#0x2700, sr

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

	/* Initialize TMSS, if relevant */
.ifndef MDK_SYSTEM_C2
	move.b	0xA10001, d0
	andi.b	#0x0F, d0
	beq	2f
	move.l	#0x53454741, 0xA14000
2:
.endif

	/* Test RAM */
	/* clear WRAM */
	move.l	#0x00FF0000, a4
	move.w	#0x8000 - 1, d7
	move.w	#0x5555, d0
wram_test_loop1:
	move.w	d0, (a4)
	cmp.w	(a4)+, d0
	bne	wram_failed
	dbra	d7, wram_test_loop1

	move.l	#0x00FF0000, a4
	move.w	#0x8000 - 1, d7
	move.w	#0xAAAA, d0
wram_test_loop2:
	move.w	d0, (a4)
	cmp.w	(a4)+, d0
	bne	wram_failed
	dbra	d7, wram_test_loop2
	bra	softreset

wram_failed:
	moveq	#7, d0
	jmp	md_error_display

softreset:
	bra	md_crt0_begin


.include	"md/asm/crt0.inc"
.include	"md/asm/dma_process.inc"
.include	"md/asm/sram.inc"

_v_bus_error:
	moveq	#0, d0
	jmp	md_error_display
_v_address_error:
	moveq	#1, d0
	jmp	md_error_display
_v_illegal_instruction:
	moveq	#2, d0
	jmp	md_error_display
_v_div_zero:
	moveq	#3, d0
	jmp	md_error_display
_v_chk:
	moveq	#4, d0
	jmp	md_error_display
_v_trapv:
	moveq	#5, d0
	jmp	md_error_display
_v_privelege:
	moveq	#6, d0
	jmp	md_error_display

_v_trace:
_v_unused_irq:
_v_aline_emu:
_v_fline_emu:
_v_reserved:
_v_coproc_violation:
_v_format:
_v_uninit:
_v_spurious:
_v_trap0x0:
_v_trap0x1:
_v_trap0x2:
_v_trap0x3:
_v_trap0x4:
_v_trap0x5:
_v_trap0x6:
_v_trap0x7:
_v_trap0x8:
_v_trap0x9:
_v_trap0xa:
_v_trap0xb:
_v_trap0xc:
_v_trap0xd:
_v_trap0xe:
_v_trap0xf:
_v_unimp:
	rte
