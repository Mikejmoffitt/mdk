	.equ VDP_DEBUG_REG_LOC, 0xC00018

	.global	md_vdp_debug_port_sel
/* void md_vdp_debug_port_sel(uint32_t num); */
md_vdp_debug_port_sel:
	move.l	4(%sp), %d0  /* num */
	lsl.w	#8, %d0
	move.w	%d0, (VDP_DEBUG_REG_LOC).l
	ori.b	#0, %d0
	rts
