	.section	.text

	.extern	g_sprite_count
	.extern	g_sprite_table
	.extern	g_sprite_next

.set	SPR_MAX, 80

#
# void md_cspr_put_st(const CSprParam *s);
#
	.global	md_cspr_put_st
.set	SPID, 4

# SP - pushed args
.set	ARG_CSPR_STRUCT, SPID

# A0 - draw params (CSprParam struct)
.set	PRM_CSPR_DATA, 0
.set	PRM_VRAM_BASE, 4
.set	PRM_FRAME, 6
.set	PRM_X, 8
.set	PRM_Y, 10
.set	PRM_ATTR, 12
.set	PRM_PRIO, 14
.set	PRM_USE_DMA, 16

# A1 - CSPR blob
.set	CSPR_NAME, 0x00
.set	CSPR_PALETTE, 0x10
.set	CSPR_REF_COUNT, 0x30
.set	CSPR_SPR_LIST_OFFSET, 0x32
.set	CSPR_TILE_DATA_OFFSET, 0x36
.set	CSPR_REFS, 0x40

# A2 - Frame ref
.set	REF_SPR_COUNT, 0
.set	REF_SPR_LIST_OFFSET, 2
.set	REF_TILE_SRC_OFFSET, 4
.set	REF_TILE_WORDS, 6

# A1 - Sprite data
.set	SPR_DY, 0
.set	SPR_SIZE, 2
.set	SPR_TILE, 4
.set	SPR_DX, 6
.set	SPR_FDY, 8
.set	SPR_RESERVED1, 10
.set	SPR_RESERVED2, 12
.set	SPR_FDX, 14

md_cspr_put_st:
# a0 := draw params (CSprParam)
	movea.l	ARG_CSPR_STRUCT(sp), a0

	movem.l	a2-a3, -(sp)
	movem.l	d2-d3/d7, -(sp)

# a1 := cspr blob
	move.l	PRM_CSPR_DATA(a0), a1

# a2 := frame ref
	lea	CSPR_REFS(a1), a2
	move.w	PRM_FRAME(a0), d0
	cmp.w	CSPR_REF_COUNT(a1), d0
	bcc	0f
	lsl.w	#3, d0  /* index by 8, sizeof(CSprRef) */
	adda.w	d0, a2  /* a2 now points to the ref */

# Sprite count checks
	move.w	REF_SPR_COUNT(a2), d7
	beq	0f  /* no sprites in this frame */

	move.w	d7, d0
	add.w	g_sprite_count, d0  /* d0 := final sprite index */
	cmpi.w	#SPR_MAX, d0  /* compare d0 to max sprites */
	bls	sprite_count_ok
	/* Too many sprites; limit amount in d7. */
	move.w	#SPR_MAX, d7  /* d7 takes max */
	sub.w	g_sprite_count, d7  /* d7 takes available slots */
	ble	0f
sprite_count_ok:

# Queue DMA if needed.
	move.w	PRM_VRAM_BASE(a0), d1
	tst.w	PRM_USE_DMA(a0)
	beq	cspr_put_no_dma

	bsr	cspr_dma_setup_sub
	bra	cspr_put_after_dma

cspr_put_no_dma:
	/* Just offset vram base */
	lsr.w	#5, d1
	add.w	REF_TILE_SRC_OFFSET(a2), d1
	bra	cspr_put_attributes_set

cspr_put_after_dma:
# d1 := base attributes for sprite
	lsr.w	#5, d1
cspr_put_attributes_set:
	or.w	PRM_ATTR(a0), d1

# a1 := spr (CSprSprite)
	adda.l	CSPR_SPR_LIST_OFFSET(a1), a1
	adda.w	REF_SPR_LIST_OFFSET(a2), a1

# d2 := sprite X
	move.w	PRM_X(a0), d2

# d3 := sprite Y
	move.w	PRM_Y(a0), d3

# a3 := MD hardware sprite slot
	movea.l	g_sprite_next, a3

	subq.w	#1, d7  /* for dbf loop */

	btst	#11, d1  /* H flip? */
	bne	cspr_hflip
	btst	#12, d1  /* V flip? */
	bne	cspr_draw_top_vflip

	.macro	cspr_draw_body_safe
	/* X pos checks */
	cmpi.w	#128+320, d2
	bcc	1f  /* skip sprite */
	cmpi.w	#128-32, d2
	bls	1f  /* skip sprite */
	/* Y pos checks */
	cmpi.w	#128+240, d3
	bcc	1f  /* skip sprite */
	cmpi.w	#128-32, d3
	bls	1f  /* skip sprite */

	/* With bounds checks done we can now write to the sprite. */
	move.w	d3, (a3)+  /* Y */
	move.b	SPR_SIZE(a1), (a3)+  /* leave link field alone */
	addq	#1, a3
	move.w	d1, d0  /* Base attr in d1 */
	add.w	SPR_TILE(a1), d0
	move.w	d0, (a3)+  /* Attr */
	move.w	d2, (a3)+  /* X */
	add.w	#1, g_sprite_count
1:
	lea	0x10(a1), a1
	.endm

cspr_draw_top_normal:
	add.w	SPR_DX(a1), d2
	add.w	SPR_DY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_normal
	bra	cspr_draw_finished

cspr_draw_top_vflip:
	add.w	SPR_DX(a1), d2
	add.w	SPR_FDY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_vflip
	bra	cspr_draw_finished

cspr_hflip:
	btst	#12, d1  /* V flip? */
	bne	cspr_draw_top_hvflip

cspr_draw_top_hflip:
	add.w	SPR_FDX(a1), d2
	add.w	SPR_DY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_hflip
	bra	cspr_draw_finished

cspr_draw_top_hvflip:
	add.w	SPR_FDX(a1), d2
	add.w	SPR_FDY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_hvflip
	bra	cspr_draw_finished

cspr_draw_finished:
0:
	move.l	a3, g_sprite_next
	movem.l	(sp)+, d2-d3/d7
	movem.l	(sp)+, a2-a3
	rts

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

	bsr	md_dma_transfer_vram
	lea	0x10(sp), sp
	movem.l	(sp)+, a0-a1
	move.w	(sp)+, d1
	rts
