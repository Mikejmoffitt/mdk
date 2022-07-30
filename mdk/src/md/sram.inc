/* mdk SRAM routiness
Michael Moffitt 2020

Thanks to Chilly Willy and Stephane Dallongeville for extant reference code.

*/

	.equ	SRAM_BASE, 0x200001
	.equ	SRAM_CTRL, 0xA130F1

	.globl	md_sram_write
md_sram_write:
	/* Enable SRAM for writing. */
	move.b	#0x01, (SRAM_CTRL).l

	/* Set up a0 with the destination address. */
	lea	SRAM_BASE, %a0
	move.l	4(%sp), %d0  /* dest_offset */
	add.l	%d0, %d0
	lea	(%a0, %d0.l), %a0

	/* a1 gets the 68000-side source address, and d1 gets the byte count. */
	move.l	8(%sp), %a1  /* src_addr */
	move.w	14(%sp), %d1  /* n */
	subi.w	#0x0001, %d1

	/* Copy n bytes. */
0:
	move.b	(%a1)+, (%a0)
	lea	2(%a0), %a0
	dbf	%d1, 0b

	/* Disable SRAM. */
	move.b	#0x00, (SRAM_CTRL).l
	rts

/* Copies n bytes from 68000 memory space src_addr to dest_offset within SRAM. */

/* Copies n bytes from src_offset in SRAM to dest_addr in 68000 memory space. */
	.globl	md_sram_read
md_sram_read:
	/* Enable SRAM (read-only). */
	move.b	#0x03, (SRAM_CTRL).l

	/* Set up a0 with the source address. */
	lea	SRAM_BASE, %a0
	move.l	4(%sp), %d0  /* src_offset */
	add.l	%d0, %d0
	lea	(%a0, %d0.l), %a0

	/* a1 gets the 68000-side dest address, and d1 gets the byte count. */
	move.l	8(%sp), %a1  /* dest_addr */
	move.w	14(%sp), %d1  /* n */
	subi.w	#0x0001, %d1

	/* Copy n bytes. */
0:
	move.b	(%a0), (%a1)+
	lea	2(%a0), %a0
	dbf	%d1, 0b

	/* Disable SRAM. */
	move.b	#0x00, (SRAM_CTRL).l
	rts

	.globl	md_sram_set
md_sram_set:
	/* Enable SRAM. */
	move.b	#0x01, (SRAM_CTRL).l

	/* Set up a0 with the destination. */
	lea	SRAM_BASE, %a0
	move.l	4(%sp), %d0  /* dest_offset */
	add.l	%d0, %d0
	lea	(%a0, %d0.l), %a0

	/* a1 gets the 68000-side dest address, and d1 gets the byte count. */
	move.w	8(%sp), %d0  /* c */
	move.w	12(%sp), %d1  /* n */
	subi.w	#0x0001, %d1

	/* Copy n bytes. */
0:
	move.b	%d0, (%a0)
	lea	2(%a0), %a0
	dbf	%d1, 0b

	/* Disable SRAM. */
	move.b	#0x00, (SRAM_CTRL).l
	rts
