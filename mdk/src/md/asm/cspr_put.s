	.section	.text

	.extern	g_sprite_count
	.extern	g_sprite_table
	.extern	g_sprite_next

	.include	"md/asm/cspr_types.inc"

.set	MD_SPR_MAX, 80

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
.set	PRM_USE_DMA, 14

md_cspr_put_st:
# a0 := draw params (CSprParam)
	movea.l	ARG_CSPR_STRUCT(sp), a0

	movem.l	a2-a3, -(sp)
	movem.l	d2-d3/d7, -(sp)

# a3 := MD hardware sprite slot
	movea.l	g_sprite_next, a3

# a1 := cspr blob
	move.l	PRM_CSPR_DATA(a0), a1

# a2 := frame ref
	lea	CSPR_REFS(a1), a2
	move.w	PRM_FRAME(a0), d0
	cmp.w	CSPR_REF_COUNT(a1), d0
	bcc.w	0f
	lsl.w	#3, d0  /* index by 8, sizeof(CSprRef) */
	adda.w	d0, a2  /* a2 now points to the ref */

# Sprite count checks
	move.w	REF_SPR_COUNT(a2), d7
	beq.w	0f  /* no sprites in this frame */

	move.w	d7, d0
	add.w	g_sprite_count, d0  /* d0 := final sprite index */
	cmpi.w	#MD_SPR_MAX, d0  /* compare d0 to max sprites */
	bls.s	sprite_count_ok
	/* Too many sprites; limit amount in d7. */
	move.w	#MD_SPR_MAX, d7  /* d7 takes max */
	sub.w	g_sprite_count, d7  /* d7 takes available slots */
	ble.w	0f
sprite_count_ok:

# Queue DMA if needed.
	move.w	PRM_VRAM_BASE(a0), d1
	tst.w	PRM_USE_DMA(a0)
	beq.s	cspr_put_no_dma

	jsr	(pc, cspr_dma_setup_sub)
	bra.s	cspr_put_after_dma

cspr_put_no_dma:
	/* Just offset vram base */
	lsr.w	#5, d1
	add.w	REF_TILE_SRC_OFFSET(a2), d1
	bra.s	cspr_put_attributes_set

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

	subq.w	#1, d7  /* for dbf loop */

	btst	#11, d1  /* H flip? */
	bne.w	cspr_hflip
	btst	#12, d1  /* V flip? */
	bne.s	cspr_draw_top_vflip

	.macro	cspr_draw_body_safe
	/* X pos checks */
	cmpi.w	#128+320, d2
	bcc.s	1f  /* skip sprite */
	cmpi.w	#128-32, d2
	bls.s	1f  /* skip sprite */
	/* Y pos checks */
	cmpi.w	#128+240, d3
	bcc.s	1f  /* skip sprite */
	cmpi.w	#128-32, d3
	bls.s	1f  /* skip sprite */

	/* With bounds checks done we can now write to the sprite. */
	move.w	d3, (a3)+  /* Y */
	move.b	SPR_SIZE(a1), (a3)+  /* leave link field alone */
	addq.l	#1, a3
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
	bra.w	cspr_draw_finished

cspr_draw_top_vflip:
	add.w	SPR_DX(a1), d2
	add.w	SPR_FDY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_vflip
	bra.w	cspr_draw_finished

cspr_hflip:
	btst	#12, d1  /* V flip? */
	bne.s	cspr_draw_top_hvflip

cspr_draw_top_hflip:
	add.w	SPR_FDX(a1), d2
	add.w	SPR_DY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_hflip
	bra.s	cspr_draw_finished

cspr_draw_top_hvflip:
	add.w	SPR_FDX(a1), d2
	add.w	SPR_FDY(a1), d3
	cspr_draw_body_safe
	dbf	d7, cspr_draw_top_hvflip
	/* fall-through to cspr_draw_finished */

cspr_draw_finished:
0:
	move.l	a3, g_sprite_next
	movem.l	(sp)+, d2-d3/d7
	movem.l	(sp)+, a2-a3
	rts
