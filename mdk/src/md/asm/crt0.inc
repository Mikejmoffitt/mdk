
md_crt0_begin:
	/* set up SP */
	move.l	(0x000000).l, sp

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
	lea	_stext, a0
	lea	0x00FF0000, a1
	move.l	#_sdata, d7

	/* last byte init fix */
	addq.l	#1, d7
	lsr.l	#1, d7
	beq	1f

	subq.w	#1, d7

.copy_var:
	move.w	(a0)+, (a1)+
	dbra	d7, .copy_var

1:

	jmp	main
