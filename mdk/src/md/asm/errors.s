#
# Error and exception handling routines.
#
#
# VDP initialization and work RAM error display routines do not use any RAM, so
# as to be suitable for a situation where work RAM is not operational.
#

.set	MD_ERROR_ATTR_ADDR,   0x0000
.set	MD_ERROR_ATTR_OK,     0x1000
.set	MD_ERROR_ATTR_TITLE,  0x2000
.set	MD_ERROR_ATTR_NG,     0x3000
.set	MD_ERROR_VRAM_TITLE,  0x3002
.set	MD_ERROR_VRAM_START,  0x3002+0xD00
.set	MD_ERROR_VRAM_REGS,   0x3002+0x500
.set	MD_ERROR_VRAM_SRPC,   0x3002+0x380
.set	MD_ERROR_VRAM_ACCESS, 0x3002+0x200

	.global	md_error_startup_wram_check_display
	.global	md_error_startup_wram_ok_display
	.global	md_error_startup_wram_ng_display
	.global	md_error_startup_start_display
	.global	md_error_exception_display

	.include	"md/asm/vdp_defs.inc"

#
# Error display entry points
#

# a6 = return point
md_error_startup_wram_check_display:
	lea	VDPPORT_DATA, a4
	/* Enable display prematurely as this function returns */
	move.w	#VDP_REGST_MODESET2 | 0x54, 4(a4)
	lea	str_wram_check, a0
	move.w	#MD_ERROR_ATTR_TITLE, d0
	move.w	#MD_ERROR_VRAM_TITLE, d1
	bra.w	string_print_sub
str_wram_check:
	.ascii	"Testing Work RAM...\0"
	.align	2

# a6 = return
md_error_startup_wram_ok_display:
	lea	VDPPORT_DATA, a4
	lea	str_wram_ok, a0
	move.w	#MD_ERROR_ATTR_OK, d0
	move.w	#MD_ERROR_VRAM_TITLE+(20*2), d1
	bra.w	string_print_sub
str_wram_ok:
	.ascii	"OK\0"
	.align	2

# a6 = return
md_error_startup_wram_ng_display:
	lea	VDPPORT_DATA, a4
	lea	str_wram_ng, a0
	move.w	#MD_ERROR_ATTR_NG, d0
	move.w	#MD_ERROR_VRAM_TITLE+(20*2), d1
	bra.w	string_print_sub
str_wram_ng:
	.ascii	"NG\0"
	.align	2

# a6 = return
md_error_startup_checksum_error_display:
	lea	VDPPORT_DATA, a4
	lea	str_checksum_error, a0
	move.w	#MD_ERROR_ATTR_TITLE, d0
	move.w	#MD_ERROR_VRAM_TITLE, d1
	bra.w	string_print_sub
str_checksum_error:
	.ascii	"ROM Checksum Error\0"
	.align	2

# a6 = return
md_error_startup_start_display:
	lea	VDPPORT_DATA, a4
	lea	str_press_start, a0
	move.w	#MD_ERROR_ATTR_NG, d0
	move.w	#MD_ERROR_VRAM_START, d1
	bra.w	string_print_sub
str_press_start:
	.ascii	"Press START to reset.\0"
	.align	2

# d0 = error type
md_error_exception_display:
	lea	VDPPORT_DATA, a4

	/* Write error message using d0 as index */
	add.w	d0, d0
	add.w	d0, d0
	lea	str_tbl(pc, d0.w), a0
	bra.w	prmsg

str_tbl:
	dc.l	str_bus_error
	dc.l	str_address_error
	dc.l	str_illegal_instruction
	dc.l	str_div_zero
	dc.l	str_chk
	dc.l	str_trapv
	dc.l	str_privelege
	dc.l	str_trace
	dc.l	str_unused_irq
	dc.l	str_aline_emu
	dc.l	str_fline_emu
	dc.l	str_reserved
	dc.l	str_coproc_violation
	dc.l	str_format
	dc.l	str_uninit
	dc.l	str_spurious
	dc.l	str_trap0x0
	dc.l	str_trap0x1
	dc.l	str_trap0x2
	dc.l	str_trap0x3
	dc.l	str_trap0x4
	dc.l	str_trap0x5
	dc.l	str_trap0x6
	dc.l	str_trap0x7
	dc.l	str_trap0x8
	dc.l	str_trap0x9
	dc.l	str_trap0xa
	dc.l	str_trap0xb
	dc.l	str_trap0xc
	dc.l	str_trap0xd
	dc.l	str_trap0xe
	dc.l	str_trap0xf
	dc.l	str_unimp

prmsg:
	cmpi.w	#4*4, d0
	bcc	md_error_format_b
md_error_format_a:
	bsr.w	md_error_print_ad_regs_sub
	bsr.w	md_error_print_access_sub
	bsr.w	md_error_print_srpc_sub
	move.w	#VDP_REGST_MODESET2 | 0x54, 4(a4)
	rts

md_error_format_b:
	bsr.w	md_error_print_ad_regs_sub
	bsr.w	md_error_print_srpc_sub
	move.w	#VDP_REGST_MODESET2 | 0x54, 4(a4)
	rts
	

/* Walks the stack and prints registers */
md_error_print_ad_regs_sub:
	/* print message */
	move.l	(a0), a0
	move.w	#MD_ERROR_ATTR_TITLE, d0
	move.w	#MD_ERROR_VRAM_TITLE, d1
	bsr.w	string_print_sub_safe
	/* Draw registers listing */
	move.w	#MD_ERROR_ATTR_OK, d0
	move.w	#MD_ERROR_VRAM_REGS, d1
	lea	str_reg_d0, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d1, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d2, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d3, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d4, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d5, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d6, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_d7, a0
	bsr.w	string_print_sub_safe

	move.w	#MD_ERROR_VRAM_REGS+0x24, d1
	lea	str_reg_a0, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a1, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a2, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a3, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a4, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a5, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a6, a0
	bsr.w	string_print_sub_safe
	add.w	#0x0100, d1
	lea	str_reg_a7, a0
	bsr.w	string_print_sub_safe
	/* Print data register contents */
	move.l	sp, a0
	addq	#8, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	move.w	#MD_ERROR_VRAM_REGS+0x08, d1
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	.rept	7
	add.w	#0x0100, d1
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	.endr
	/* And address registers */
	move.w	#MD_ERROR_ATTR_ADDR, d0
	move.w	#MD_ERROR_VRAM_REGS+0x24+0x08, d1
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	.rept	7
	add.w	#0x0100, d1
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	.endr
	rts

# a0 = pointer to sstart of format b stack frame
md_error_print_srpc_sub:
	/* SR */
	move.l	a0, -(sp)
	move.w	#MD_ERROR_ATTR_OK, d0
	move.w	#MD_ERROR_VRAM_SRPC, d1
	lea	str_reg_sr, a0
	bsr.w	string_print_sub_safe
	move.l	(sp)+, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	addq	#8, d1
	moveq	#0, d3
	move.w	(a0)+, d3
	bsr.w	hex_print_word_sub_safe
	/* PC */
	move.l	a0, -(sp)
	move.w	#MD_ERROR_ATTR_OK, d0
	move.w	#MD_ERROR_VRAM_SRPC+0x24, d1
	lea	str_reg_pc, a0
	bsr.w	string_print_sub_safe
	move.l	(sp)+, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	addq	#8, d1
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	rts

# a0 = pointer to sstart of format b stack frame
md_error_print_access_sub:
	/* flags */
	move.l	a0, -(sp)
	move.w	#MD_ERROR_ATTR_NG, d0
	move.w	#MD_ERROR_VRAM_ACCESS, d1
	lea	str_acc, a0
	bsr.w	string_print_sub_safe
	move.l	(sp)+, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	add.w	#10, d1
	moveq	#0, d3
	move.w	(a0)+, d3
	bsr.w	hex_print_byte_sub_safe
	/* address */
	add.w	#6, d1
	move.l	a0, -(sp)
	move.w	#MD_ERROR_ATTR_NG, d0
	lea	str_at, a0
	bsr.w	string_print_sub_safe
	move.l	(sp)+, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	addq	#8, d1
	moveq	#0, d3
	move.l	(a0)+, d3
	bsr.w	hex_print_long_sub_safe
	/* IR */
	add.w	#18, d1
	move.l	a0, -(sp)
	move.w	#MD_ERROR_ATTR_NG, d0
	lea	str_reg_ir, a0
	bsr.w	string_print_sub_safe
	move.l	(sp)+, a0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	move.w	#MD_ERROR_ATTR_ADDR, d0
	addq	#8, d1
	move.w	(a0)+, d3
	bsr.w	hex_print_word_sub_safe
	rts

#
# Printing routines
#
# String print requiring no RAM.
# d0 = attr (palette, etc)
# d1 = vram address
# d2 = clobbered
# d3 = data
# a4 = VDPPORT_DATA
# a6 = return address
hex_print_long_sub:
	/* Move VRAM to end of number */
	add.w	#(8-1)*2, d1
	/* Loop for eight digits */
	move.w	#8-1, d7
hex_print_sub_main:
	move.w	#VDP_REGST_AUTOINC | 0x80, 4(a4)
	/* Set up d0 to hold attr data in upper word */
	swap	d0
	clr.w	d0
hex_print_sub_loop:
	move.w	d1, -(sp)
	/* VRAM set */
	moveq	#0, d2
	move.w	d1, d2
	andi.w	#0x3FFF, d2
	swap	d2
	andi.w	#0xC000, d1
	lsr.w	#8, d1
	lsr.w	#6, d1
	move.w	d1, d2
	ori.l	#VRAM_ADDR_CMD, d2
	move.l	d2, 4(a4)
	move.w	(sp)+, d1
	subi.w	#2, d1
	/* Number print */
	move.b	d3, d2
	andi.w	#0x000F, d2
	move.b	hex_digit_tbl(pc, d2.w), d0
	move.w	d0, (a4)
	addi.w	#0x10, d0
	move.w	d0, (a4)
	lsr.l	#4, d3
	dbf	d7, hex_print_sub_loop
	jmp	(a6)

hex_digit_tbl:
	dc.b	0x20
	dc.b	0x21
	dc.b	0x22
	dc.b	0x23
	dc.b	0x24
	dc.b	0x25
	dc.b	0x26
	dc.b	0x27
	dc.b	0x28
	dc.b	0x29
	dc.b	0x41
	dc.b	0x42
	dc.b	0x43
	dc.b	0x44
	dc.b	0x45
	dc.b	0x46

hex_print_word_sub:
	/* Move VRAM to end of number */
	add.w	#(4-1)*2, d1
	/* Loop for eight digits */
	move.w	#4-1, d7
	bra.w	hex_print_sub_main

hex_print_byte_sub:
	/* Move VRAM to end of number */
	add.w	#(2-1)*2, d1
	/* Loop for eight digits */
	move.w	#2-1, d7
	bra.w	hex_print_sub_main

hex_print_long_sub_safe:
	lea	hex_long_safe_ret, a6
	movem.l	d0-d3/a0-a2, -(sp)
	bra.w	hex_print_long_sub
hex_long_safe_ret:
	movem.l	(sp)+, d0-d3/a0-a2
	rts
hex_print_word_sub_safe:
	lea	hex_long_safe_ret, a6
	movem.l	d0-d3/a0-a2, -(sp)
	bra.w	hex_print_word_sub
hex_print_byte_sub_safe:
	lea	hex_long_safe_ret, a6
	movem.l	d0-d3/a0-a2, -(sp)
	bra.w	hex_print_byte_sub

# Prints a longword in hexidecimal.

# Wrapper for string_print_sub that protects registers and uses the stack.
string_print_sub_safe:
	lea	str_safe_ret, a6
	movem.l	d0-d3/a0-a2, -(sp)
	bra.w	string_print_sub
str_safe_ret:
	movem.l	(sp)+, d0-d3/a0-a2
	rts

# String print requiring no RAM.
# a0 = C string (null terminated)
# d0 = attr (palette, etc)
# d1 = vram address
# a4 = VDPPORT_DATA
# a6 = return address
string_print_sub:
	move.w	#VDP_REGST_AUTOINC | 0x02, 4(a4)
	/* Set VRAM address */
	move.l	d1, d2
	andi.w	#0x3FFF, d2
	swap	d2
	andi.w	#0xC000, d1
	lsr.w	#8, d1
	lsr.w	#6, d1
	move.w	d1, d2
	ori.l	#VRAM_ADDR_CMD, d2
	move.l	d2, d3
	move.l	d2, 4(a4)
	move.l	a0, a2
str_print1:
	move.b	(a0)+, d0
	beq	str_print1_done
	sub.b	#0x20, d0
	move.w	d0, d1
	andi.w	#0xFFF0, d1
	add.w	d0, d1
	move.w	d1, (a4)
	bra.w	str_print1
str_print1_done:
	/* Set VRAM address again */
	swap	d2
	addi.w	#0x80, d2  /* Next row */
	swap	d2
	move.l	d2, 4(a4)
str_print2:
	move.b	(a2)+, d0
	beq	str_print2_done
	sub.b	#0x20, d0
	move.w	d0, d1
	andi.w	#0xFFF0, d1
	add.w	d0, d1
	add.b	#0x10, d1
	move.w	d1, (a4)
	bra.w	str_print2
str_print2_done:
	jmp	(a6)

#
# String data
#

str_bus_error:
	.ascii	"Bus Error\0"
str_address_error:
	.ascii	"Address Error\0"
str_illegal_instruction:
	.ascii	"Illegal Instruction\0"
str_div_zero:
	.ascii	"Zero Divide\0"
str_chk:
	.ascii	"CHK Exception\0"
str_trapv:
	.ascii	"TrapV Exception\0"
str_privelege:
	.ascii	"Priveleged Instruction\0"
str_vram_error:
	.ascii	"Video RAM Error\0"
str_trace:
	.ascii	"Trace (Unimplemented)\0"
str_unused_irq:
	.ascii	"Unused IRQ\0"
str_aline_emu:
	.ascii	"A Line Emu\0"
str_fline_emu:
	.ascii	"F Line Emu\0"
str_reserved:
	.ascii	"Reserved\0"
str_coproc_violation:
	.ascii	"Coproc Violation\0"
str_format:
	.ascii	"Format\0"
str_uninit:
	.ascii	"Uninit\0"
str_spurious:
	.ascii	"Spurious Interrupt\0"
str_trap0x0:
	.ascii	"Trap $0\0"
str_trap0x1:
	.ascii	"Trap $1\0"
str_trap0x2:
	.ascii	"Trap $2\0"
str_trap0x3:
	.ascii	"Trap $3\0"
str_trap0x4:
	.ascii	"Trap $4\0"
str_trap0x5:
	.ascii	"Trap $5\0"
str_trap0x6:
	.ascii	"Trap $6\0"
str_trap0x7:
	.ascii	"Trap $7\0"
str_trap0x8:
	.ascii	"Trap $8\0"
str_trap0x9:
	.ascii	"Trap $9\0"
str_trap0xa:
	.ascii	"Trap $A\0"
str_trap0xb:
	.ascii	"Trap $B\0"
str_trap0xc:
	.ascii	"Trap $C\0"
str_trap0xd:
	.ascii	"Trap $D\0"
str_trap0xe:
	.ascii	"Trap $E\0"
str_trap0xf:
	.ascii	"Trap $f\0"
str_unimp:
	.ascii	"Unimplemented\0"

str_reg_d0:
	.ascii	"D0 $\0"
str_reg_d1:
	.ascii	"D1 $\0"
str_reg_d2:
	.ascii	"D2 $\0"
str_reg_d3:
	.ascii	"D3 $\0"
str_reg_d4:
	.ascii	"D4 $\0"
str_reg_d5:
	.ascii	"D5 $\0"
str_reg_d6:
	.ascii	"D6 $\0"
str_reg_d7:
	.ascii	"D7 $\0"
str_reg_a0:
	.ascii	"A0 $\0"
str_reg_a1:
	.ascii	"A1 $\0"
str_reg_a2:
	.ascii	"A2 $\0"
str_reg_a3:
	.ascii	"A3 $\0"
str_reg_a4:
	.ascii	"A4 $\0"
str_reg_a5:
	.ascii	"A5 $\0"
str_reg_a6:
	.ascii	"A6 $\0"
str_reg_a7:
	.ascii	"A7 $\0"

str_reg_sp:
	.ascii	"SP $\0"
str_reg_usp:
	.ascii	"USP $\0"
str_reg_pc:
	.ascii	"PC $\0"
str_reg_sr:
	.ascii	"SR $\0"
str_acc:
	.ascii	"Acc $\0"
str_at:
	.ascii	"at $\0"
str_reg_ir:
	.ascii	"IR $\0"

	.align	2
