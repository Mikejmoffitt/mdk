.set	Z80_PRG_LOC,   0xA00000
.set	Z80_BUS_LOC,   0xA11100
.set	Z80_RESET_LOC, 0xA11200

# bus req (no wait)
	.macro	zbusreq
	move.b	#0x01, Z80_BUS_LOC
	.endm

# bus release
	.macro	zbusrel
	clr.b	Z80_BUS_LOC
	.endm

# bus wait
	.macro	zbuswt
9:	btst	#0, Z80_BUS_LOC
	bne	9b
	.endm
