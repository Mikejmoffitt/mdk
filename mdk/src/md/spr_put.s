# Sprite placement functions.

	.extern	g_sprite_count
	.extern	g_sprite_table

.set	SPR_STATIC_OFFS, 128
.set	SPR_MAX, 80

#
# void md_spr_put(int16_t x, int16_t y, uint16_t attr, uint16_t size)
#
	.global	md_spr_put
md_spr_put:
SPID = 4

# All arguments are word-sized, while aligned to longword displacements.
.set	ARG_X, SPID+2
.set	ARG_Y, SPID+(4*1)+2
.set	ARG_ATTR, SPID+(4*2)+2
.set	ARG_SIZE, SPID+(4*3)+2
# Check if sprite count hasn't been exceeded.
	move.w	g_sprite_count, d0
	cmpi.w	#SPR_MAX, d0
	bcc	0f
# Check if X coordinates are out of frame.
	swap	d0                         /* Save sprite index for later. */
	move.w	ARG_X(sp), d0              /* X position argument. */
	addi.w	#SPR_STATIC_OFFS, d0       /* Offset for screen space. */
	andi.w	#0x01FF, d0                /* Mask 9 bits valid for VDP. */
	cmpi.w	#SPR_STATIC_OFFS-32, d0    /* Check for sprites too far off */
	bcs	0f                         /* screen to avoid line mask. */
# Set A0 to point at sprite slot and store updated sprite count.
	swap	d0
	move.w	d0, d1
	addq	#1, d1
	move.w	d1, g_sprite_count

	lea	g_sprite_table, a0
	lsl.w	#3, d0  /* d0 *= sizeof(SprSlot) */
	adda.w	d0, a0  /* a0 now points to sprite table entry. */

# Y position and link.
	move.w	ARG_Y(sp), d1
	addi.w	#SPR_STATIC_OFFS, d1
	move.w	d1, (a0)+
	move.w	ARG_SIZE(sp), d1  /* only lower byte is actually used */
	move.b	d1, (a0)         /* to avoid overwriting link field. */
# Attribute, X position.
	move.w	ARG_ATTR(sp), d0
	swap	d0  /* d0 how has attr in high word and x in low word. */
	move.l	d0, 2(a0)
0:
	rts
