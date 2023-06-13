	.section	.text
	.global		kosinski_decomp

# Courtesy of https://segaretro.org/Kosinski_compression

kosinski_decomp:
.set	SPID, 4
.set	ARG_SRC, SPID
.set	ARG_DST, SPID+(4*1)
	movea.l	ARG_SRC(sp), a0
	movea.l	ARG_DST(sp), a1
	movem.l	d2-d6, -(sp)


kos_decomp:
	subq.l	#2, sp    /* Two bytes on the stack */
	move.b	(a0)+, 1(sp)
	move.b	(a0)+, (sp)
	move.w	(sp), d5  /* Copy first desc field */
	moveq	#0xF, d4  /* 16 bits in a byte */

kos_decomp_loop:
	lsr.w	#1, d5  /* Bit shifted out goes into C flag */
	move.w	sr, d6
	dbf	d4, kos_decomp_chkbit
	move.b	(a0)+, 1(sp)
	move.b	(a0)+, (sp)
	move.w	(sp), d5  /* Get next desc field if needed */
	moveq	#0xF, d4  /* Reset bit counter */

kos_decomp_chkbit:
	move.w	d6, ccr  /* was the bit set? */
	bcc	kos_decomp_match  /* if not, branch (C flag = bit clear */
	move.b	(a0)+, (a1)+  /* otherwise, copy byte as-is */
	bra	kos_decomp_loop

# ------------------------------------------------------------------------------

kos_decomp_match:
	moveq	#0, d3
	lsr.w	#1, d5  /* get next bit */
	move.w	sr, d6
	dbf	d4, kos_decomp_chkbit2
	move.b	(a0)+, 1(sp)
	move.b	(a0)+, (sp)
	move.w	(sp), d5
	moveq	#0xF, d4

kos_decomp_chkbit2:
	move.w	d6, ccr  /* bit set? */
	bcs	kos_decomp_fullmatch  /* if it was, branch */
	lsr.w	#1, d5  /* bit shifted out goes to X flag */

	dbf	d4, 1f
	move.b	(a0)+, 1(sp)
	move.b	(a0)+, (sp)
	move.w	(sp), d5
	moveq	#0xF, d4
1:
	roxl.w	#1, d3  /* get high repeat count bit (shift x flag in) */
	lsr.w	#1, d5

	dbf	d4, 2f
	move.b	(a0)+, 1(sp)
	move.b	(a0)+, (sp)
	move.w	(sp), d5
	moveq	#0xF, d4
2:
	roxl.w	#1, d3  /* get low repeat count bit */
	addq.w	#1, d3  /* increment repeat count */
	moveq	#-1, d2
	move.b	(a0)+, d2  /* calculate offset */
	bra	kos_decomp_matchloop

kos_decomp_fullmatch:
	move.b	(a0)+, d0  /* first byte */
	move.b	(a0)+, d1  /* second byte */
	moveq	#-1, d2
	move.b	d1, d2
	lsl.w	#5, d2
	move.b	d0, d2  /* calculate offset */
	andi.w	#7, d1  /* third byte needed? */
	beq.s	kos_decomp_fullmatch2  /* if so, branch */
	move.b	d1, d3  /* copy repeat count */
	addq.w	#1, d3  /* and increment it */

kos_decomp_matchloop:
	move.b	(a1, d2.w), d0
	move.b	d0, (a1)+                 /* copy appropriate byte */
	dbf	d3, kos_decomp_matchloop  /* and repeat the copy */
	bra	kos_decomp_loop

kos_decomp_fullmatch2:
	move.b	(a0)+, d1
	beq	kos_decomp_done  /* 0 indicates end of stream */
	cmpi.b	#1, d1
	beq	kos_decomp_loop  /* 1 indicates new desc needed */
	move.b	d1, d3  /* else, copy repeat count */
	bra	kos_decomp_matchloop
kos_decomp_done:
	addq.l	#2, sp  /* restore from two scratch bytes */

	movem.l	(sp)+, d2-d6
	rts
