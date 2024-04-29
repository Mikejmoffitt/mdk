	.global	md_tmss_init

md_tmss_init:
	/* poke TMSS */
	move.b	0xa10001, d0
	andi.b	#0x0F, d0
	beq.s	.tmss_version_0
	move.l	#0x53454741, 0xa14000
.tmss_version_0:
	rts
