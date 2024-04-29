
md_crt0_begin:
	/* clear WRAM */
	lea	(0xFFFF0000), a0
	move.w	#(0x10000/4)-1, d7
	moveq	#0, d0
.clr_loop:
	move.l	d0, (a0)+
	dbra	d7, .clr_loop

	/* copy data to work RAM */
	lea	_stext, a0  /* Data follows text section */
	lea	0xFFFF0000, a1
	move.w	#_sdata, d0
	beq.s	.no_data_copy
	lsr.w	#1, d0
	subq.w	#1, d0

.copy_data:
	move.w	(a0)+, (a1)+
	dbra	d0, .copy_data

.no_data_copy:
	jmp	(main).l
