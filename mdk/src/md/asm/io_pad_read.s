/* ASM routines supporting io.h. */

	.equ SYS_Z80_PORT_BUS, 0xA11100
	.equ IO_LOC_VERSION, 0xA10000
	.equ IO_LOC_DATA,    0xA10003
	.equ IO_LOC_CTRL,    0xA10009
	.equ IO_LOC_CTRL_L,  0xA10008

	/* uint16_t[2] */
	.extern	g_md_pad[2]
	.extern	g_md_pad_pos[2]
	.extern	g_md_pad_neg[2]
	.extern	g_md_pad_prev[2]

/* a0 points to pad cache in RAM we are updating.
   a1 points to the pad data port (byte address). */
	.global	md_io_read_pad
md_io_read_pad:
	/* First step - x1CBRLDU into d0 */
	move.b	#0x40, (a1)  /* TH hi */
	nop
	nop
	nop
	move.b	(a1), d0
	/* Second step - x0SA00DU into d1 */
	move.b	#0x00, (a1)  /* TH low */
	ori.w	#0xFFC0, d0
	nop
	move.b	(a1), d1
	/* If the pad doesn't positively identify as an MD pad (bits 2 and 3
	   cleared), then don't proceed to read later phases. */
	/* TODO: Gracefully handle peripherals like Mega Mouse, etc. */
	btst.b	#2, d1
	bne	store_pad_unplugged
	btst.b	#3, d1
	bne	store_pad_unplugged
	/* Proceed to use the second phase data. */
	lsl.b	#2, d1
	ori.w	#0xFF3F, d1
	and.w	d1, d0
	/* d0 contains a full 3-button controller's worth now. */
	/* Third step - write TH high and disregard data. */
	/* Fifth step - write TH high and disregard data. */
	move.b	#0x40, (a1)  /* TH hi */
	nop
	nop
	nop
	/* Sixth step - write TH low, and use lower nybble to check pad type. */
	move.b	#0x00, (a1)  /* TH low */
	nop
	nop
	nop
	move.b	(a1), d1
	/* If the lower four bits are non-zero, it's not a 6-button pad. */
	/* Seventh and final step - write TH high, read extra buttons */
	move.b	#0x40, (a1)  /* TH hi */
	andi.b	#0x0F, d1
	bne.s	store_pad
	move.b	(a1), d1
	/* Shift buttons into place in upper byte and mask off other bits */
	lsl.w	#8, d1
	ori.w	#0xF0FF, d1
	and.w	d1, d0
	/* Mark highest bit to indicate a 6-button controller. */
	bclr	#15, d0
	bra.s	store_pad

store_pad_unplugged:
	/* Mark second-highest bit for "unplugged/maybe sms" status. */
	bclr	#14, d0
	/* Fall-through intended. */
store_pad:
	/* Invert buttons for easier use with the C MdButton enum. */
	not.w	d0
	move.w	d0, (a0)
	move.b	#0x00, (a1)  /* TH back to low */
	rts

	.global	md_io_init
md_io_init:
	clr.l	d0
	move.l	d0, g_md_pad
	move.l	d0, g_md_pad_prev
	move.l	d0, g_md_pad_pos
	move.l	d0, g_md_pad_neg
	move.l	#0x00400040, (IO_LOC_CTRL_L).l  /* Set TH pin as an output */
	rts

	.global	md_io_poll
md_io_poll:
	/* Move stale pad data into previous frame var. */
	move.l	g_md_pad, g_md_pad_prev

	/* Pause Z80 before we touch the ports. */
	move.w	#0x0100, (SYS_Z80_PORT_BUS).l  /* Z80 Bus Request */
.bus_req_wait:
	btst.b	#0, (SYS_Z80_PORT_BUS).l
	bne.s	.bus_req_wait

	lea	g_md_pad, a0
	lea	IO_LOC_DATA, a1
	bsr.w	md_io_read_pad
	lea	g_md_pad + 2, a0
	lea	IO_LOC_DATA + 2, a1
	bsr.w	md_io_read_pad

	/* Done with the ports, so release the Z80. */
	move.w	#0x0000, (SYS_Z80_PORT_BUS).l  /* Z80 Bus Release */

	/* Fall-through to md_io_generate_edges */

	.global	md_io_generate_edges
md_io_generate_edges:
	/* Generate pos and neg edge data. */
	move.l	g_md_pad, d0
	move.l	g_md_pad_prev, d1
	eori.l	#0xFFFFFFFF, d1
	and.l	d1, d0
	move.l	d0, g_md_pad_pos

	move.l	g_md_pad_prev, d0
	move.l	g_md_pad, d1
	eori.l	#0xFFFFFFFF, d1
	and.l	d1, d0
	move.l	d0, g_md_pad_neg
	rts
