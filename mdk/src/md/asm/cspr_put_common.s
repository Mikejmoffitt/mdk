	.include	"md/asm/cspr_types.inc"

	.global	cspr_dma_setup_sub
cspr_dma_setup_sub:
	move.w	d1, -(sp)
	movem.l	a0-a1, -(sp)
	moveq	#2, d0
	move.l	d0, -(sp)  /* stride */

	move.w	REF_TILE_WORDS(a2), d0
	move.l	d0, -(sp)  /* words */

	adda.l	CSPR_TILE_DATA_OFFSET(a1), a1
	moveq	#0, d0
	move.w	REF_TILE_SRC_OFFSET(a2), d0
	lsl.l	#5, d0  /* 32 bytes per tile */
	adda.l	d0, a1
	move.l	a1, -(sp)  /* source data */

	moveq	#0, d0
	move.w	d1, d0
	move.l	d0, -(sp)  /* dest vram */

	jsr	(pc, md_dma_transfer_vram)
	lea	0x10(sp), sp
	movem.l	(sp)+, a0-a1
	move.w	(sp)+, d1
	rts
