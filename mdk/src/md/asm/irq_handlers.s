/* md-toolchain interrupt handlers
Michael Moffitt 2018-2022
*/

	.global	_v_irq1
	.global	_v_irq2
	.global	_v_irq3
	.global	_v_irq4
	.global	_v_irq5
	.global	_v_irq6
	.global	_v_irq7

/* This is a flag used for vblank waiting. */
	.extern g_vblank_wait

/* Pointer to interrupt routines. Calling convention is just a standard void
   function which takes no arguments (e.g. my_irq_handler(void)).
   As the 68000 has a 24-bit address bus, the upper eight bits of a pointer can
   not be physically expressed by the CPU and thus can be (ab)used for flags.
   The highest bit, #31, is being used as a flag to indicate that the function
   will not clobber d0-d1/a0-a1 and thus doesn't need protection. */
	.extern g_irq_h_func
	.extern g_irq_io_func
	.extern g_irq_v_func

	.macro	irq_func_handle symbol_name:req
	/* Disable interrupts. Don't touch the global flag for it, though.*/
	ori.w	#0x0700, sr
	/* Load handler, and nope out if nothing has been registered. */
	tst.l	\symbol_name
	beq	2f
	/* If the top bit is set, this routine won't clobber d0-d1/a0-a1. */
	btst.b	#7, \symbol_name
	bne	1f
	/* Otherwise, call it with clobber protection (a little slower!) */
	movem.l	d0-d1/a0-a1, -(sp)
	move.l	\symbol_name, a0
	jsr	(a0)
	movem.l	(sp)+, d0-d1/a0-a1
	bra	2f
/* For functions that do not clobber. */
1:
	move.l	a0, -(sp)  /* a0 is used so we have to save it */
	move.l	\symbol_name, a0
	jsr	(a0)
	move.l	(sp)+, a0  /* Restore a0 after. */
2:
	.endm

/* IRQ vectors unused */
_v_irq1:
_v_irq3:
_v_irq5:
_v_irq7:
	rte

/* /TH pin change, from controller port */
_v_irq2:
	irq_func_handle g_irq_io_func
	rte

/* H-blank Interrupt */
_v_irq4:
	irq_func_handle g_irq_h_func
	rte

/* V-blank Interrupt */
_v_irq6:
	/*
	This flag is cleared in vblank so that software may set it high,
	and then block on it transitioning low in order to synchronize.
	*/
	clr.w	g_vblank_wait
	irq_func_handle g_irq_v_func
	rte
