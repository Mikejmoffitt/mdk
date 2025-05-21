#pragma once
#include	"md/mmio.h"

# bus req (no wait)
	.macro	zbusreq
	move.b	#0x01, SYS_Z80_PORT_BUS_LOC
	.endm

# bus release
	.macro	zbusrel
	sf	SYS_Z80_PORT_BUS_LOC
	.endm

# bus wait
	.macro	zbuswt
9:	btst	#0, SYS_Z80_PORT_BUS_LOC
	bne	9b
	.endm
