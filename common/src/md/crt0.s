/* md-toolchain header, interrupts, and C runtime startup
Michael Moffitt 2021 */

	.section	.text.keepboot

	.extern	_etext
	.extern	_stext
	.extern	_edata
	.extern	_sdata
	.extern	main

	.extern _v_irq1
	.extern _v_irq2
	.extern _v_irq3
	.extern _v_irq4
	.extern _v_irq5
	.extern _v_irq6
	.extern _v_irq7
	.extern megadrive_init

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

	/* Sega Megadrive / Genesis header */
	/* Console name */
	/* 16:   ________________ */
	.ascii	"SEGA MEGA DRIVE "
	/* Copyright information */
	/* 16:   ________________ */
	.ascii	"(C)MOFT 2020.AUG"
	/* Domestic name*/
	/* 48:   ________________________________________________ */
	.ascii	"LYLE IN CUBE SECTOR                             "
	/* Overseas name*/
	/* 48:   ________________________________________________ */
	.ascii	"LYLE IN CUBE SECTOR                             "
	/* Serial number */
	/* 16:   _______________*/
	.ascii	"GM 68000420-69"
	/* Checksum (2 bytes) */
	.short	0x0000
	/* I/O Support */
	/* 16:   ________________ */
	.ascii	"JD              "
	/* ROM start and end */
	.long	0
	.long	0x001FFFFF
	/* Work RAM start and end */
	.long	0x000FF000
	.long	0x000FFFFF
	/* Backup memory */
	.ascii	"RA"
	.byte	0xF8
	.byte	0x20
	/* Backup RAM start and end*/
	.long	0x200001
	.long	0x20FFFF
	/* Modem */
	/* 12:   ____________ */
	.ascii	"            "
	/* Reserved */
	/* 40:   ________________________________________ */
	.ascii	"                                        "
	/* Country codes */
	.ascii	"JUE"
	/* MReserved */
	/* 13:   _____________ */
	.ascii	"             "

_start:
start:
	/* disable ints */
	move.w	#0x2700, %sr

	/* set up SP */
	move.l	0x00000000, %sp

	/* poke TMSS */
	move.b	0xa10001, %d0
	andi.b	#0x0F, %d0
	beq	.tmss_version_0
	move.l	#0x53454741, 0xa14000
.tmss_version_0:

	/* clear WRAM */
	move.l	#0x00FF0000, %a4
	move.w	#0x3FFF, %d7
	moveq	#0, %d0
.clr_loop:
	move.l	%d0, (%a4)+
	dbra	%d7, .clr_loop

	/* copy data to work RAM */
	lea	_stext, %a0
	lea	0x00FF0000, %a1
	move.l	#_sdata, %d7

	/* last byte init fix */
	addq.l	#1, %d7
	lsr.l	#1, %d7
	beq	.no_copy

	subq.w	#1, %d7

.copy_var:
	move.w	(%a0)+, (%a1)+
	dbra	%d7, .copy_var

.no_copy:

	jmp	main

	.global	softreset
softreset:
	move.l	#0xFFFFE0, %sp
	jsr	megadrive_init
	jmp	start

.include	"md/irq.inc"
.include	"md/io.inc"
.include	"md/sram.inc"

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
