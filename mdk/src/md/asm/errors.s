	.global	md_error_display
	.include	"md/asm/vdp_defs.inc"

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
	

# d0 = error type
md_error_display:
	movem.l	d0-d7/a0-a7, -(sp)
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

	/* Write error message using d0 as index */
	add.w	d0, d0
	add.w	d0, d0
	lea	str_tbl(pc, d0.w), a0
	bra	prmsg

str_tbl:
	dc.l	str_bus_error
	dc.l	str_address_error
	dc.l	str_illegal_instruction
	dc.l	str_div_zero
	dc.l	str_chk
	dc.l	str_trapv
	dc.l	str_privelege
	dc.l	str_wram_error
	dc.l	str_checksum_error

prmsg:
	cmpi.w	#7*4, d0
	bcs	full_regdump

	/* just print the message */
	move.l	(a0), a0
	move.w	#0x2000, d0
	move.w	#0x3082, d1
	lea	enable_display, a6
	bra	string_print_a0

full_regdump:
	/* print message */
	move.l	(a0), a0
	move.w	#0x2000, d0
	move.w	#0x3082, d1
	bsr	string_print_a0_safe
	/* Draw registers listing */
	moveq	#0, d0
	move.w	#0x3482, d1
	lea	str_reg_d0, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d1, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d2, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d3, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d4, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d5, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d6, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_d7, a0
	bsr	string_print_a0_safe
	move.w	#0x3482+0x24, d1
	lea	str_reg_a0, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a1, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a2, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a3, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a4, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a5, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a6, a0
	bsr	string_print_a0_safe
	add.w	#0x0100, d1
	lea	str_reg_a7, a0
	bsr	string_print_a0_safe
post_reglabels:

enable_display:

	/* Enable display */
	move.w	#VDP_REGST_MODESET2 | 0x44, (a5)

forever:
	bra	forever

# Wrapper for string_print_a0 that protects registers and uses the stack.
string_print_a0_safe:
	lea	a0_safe_ret, a6
	movem.l	d0-d3/a0-a2, -(sp)
	bra	string_print_a0
a0_safe_ret:
	movem.l	(sp)+, d0-d3/a0-a2
	rts
	

# String print requiring no RAM.
# a0 = C string (null terminated)
# d0 = attr (palette, etc)
# d1 = vram address
# a6 = return address
string_print_a0:
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
	move.l	d2, (VDPPORT_CTRL)
	move.l	a0, a2
str_print1:
	move.b	(a0)+, d0
	beq	str_print1_done
	sub.b	#0x20, d0
	move.w	d0, d1
	andi.w	#0xFFF0, d1
	add.w	d0, d1
	move.w	d1, (a4)
	bra	str_print1
str_print1_done:
	/* Set VRAM address again */
	swap	d2
	addi.w	#0x80, d2  /* Next row */
	swap	d2
	move.l	d2, (VDPPORT_CTRL)
str_print2:
	move.b	(a2)+, d0
	beq	str_print2_done
	sub.b	#0x20, d0
	move.w	d0, d1
	andi.w	#0xFFF0, d1
	add.w	d0, d1
	add.b	#0x10, d1
	move.w	d1, (a4)
	bra	str_print2
str_print2_done:
	jmp	(a6)
	

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
str_wram_error:
	.ascii	"Work RAM Error\0"
str_vram_error:
	.ascii	"Video RAM Error\0"
str_checksum_error:
	.ascii	"ROM Checksum Error\0"

str_reg_d0:
	.ascii	"D0\0"
str_reg_d1:
	.ascii	"D1\0"
str_reg_d2:
	.ascii	"D2\0"
str_reg_d3:
	.ascii	"D3\0"
str_reg_d4:
	.ascii	"D4\0"
str_reg_d5:
	.ascii	"D5\0"
str_reg_d6:
	.ascii	"D6\0"
str_reg_d7:
	.ascii	"D7\0"
str_reg_a0:
	.ascii	"A0\0"
str_reg_a1:
	.ascii	"A1\0"
str_reg_a2:
	.ascii	"A2\0"
str_reg_a3:
	.ascii	"A3\0"
str_reg_a4:
	.ascii	"A4\0"
str_reg_a5:
	.ascii	"A5\0"
str_reg_a6:
	.ascii	"A6\0"
str_reg_a7:
	.ascii	"A7\0"

	.align	2

md_error_palette:
# white-cyan
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
# yellow-green
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
# cyan-blue
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
# pink-blue
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
