
md_crt0_begin:
	/* clear WRAM */
	move.l	#0x00FF0000, a4
	move.w	#0x07FF, d7
	moveq	#0, d0
.clr_loop:
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	move.l	d0, (a4)+
	dbra	d7, .clr_loop

	/* copy data to work RAM */
	lea	_stext, a0  /* Data follows text section */
	lea	0x00FF0000, a1
	move.w	#_sdata, d7
	beq	.no_data_copy
	lsr.w	#1, d7
	subq	#1, d7

.copy_data:
	move.w	(a0)+, (a1)+
	dbra	d7, .copy_data

.no_data_copy:

	/* set up SP */
	move.l	(0x01000000).l, sp

	jmp	main
