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
	/* access fault */
	.long	_v_access_fault
	/* access error */
	.long	_v_access_error
	/* illegal instruction */
	.long	_v_illegal_instruction
	/* divide by zero error */
	.long	_v_di_v_zero
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
	/* disable ints */
	move.w	#0x2700, sr

	/* set up SP */
	move.l	(0x000000).l, sp

	/* clear WRAM */
	move.l	#0x00FF0000, a4
	move.w	#0x3FFF, d7
	moveq	#0, d0
.clr_loop:
	move.l	d0, (a4)+
	dbra	d7, .clr_loop

	/* copy data to work RAM */
	lea	_stext, a0
	lea	0x00FF0000, a1
	move.l	#_sdata, d7

	/* last byte init fix */
	addq.l	#1, d7
	lsr.l	#1, d7
	beq	.no_copy

	subq.w	#1, d7

.copy_var:
	move.w	(a0)+, (a1)+
	dbra	d7, .copy_var

.no_copy:
	move.l	(0x000000).l, sp
	jmp	main

	.global	softreset
softreset:

	jmp	start

/* Code is included here explicitly so it is always in low ROM. */
.include	"md/asm/dma_process.inc"
.include	"md/asm/sram.inc"

_v_access_fault:
_v_access_error:
_v_illegal_instruction:
_v_di_v_zero:
_v_chk:
_v_trapv:
_v_privelege:
	jmp	softreset

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
