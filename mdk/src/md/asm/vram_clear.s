	.global	md_vdp_vram_clear
md_vdp_vram_clear:
	/* Clear VRAM */
	lea	(0xC00000).l, a0  /* VDP */
	move.w	#0x8F02, 4(a0)  /* Autoinc of 2 bytes */
	moveq	#0, d0
	move.l	#0x40000000, 4(a0)  /* VRAM address 0, write */
	move.w	#(0x10000/16) - 1, d1
.clear_top:
	move.l	d0, (a0)
	move.l	d0, (a0)
	move.l	d0, (a0)
	move.l	d0, (a0)
	dbra	d1, .clear_top
	rts
