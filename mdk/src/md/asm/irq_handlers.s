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
   function which takes no arguments (e.g. my_irq_handler(void)). */
	.extern g_irq_h_func
	.extern g_irq_io_func
	.extern g_irq_v_func

	.macro	irq_func_handle symbol_name:req
	/* Disable interrupts. Don't touch the global flag for it, though.*/
	ori.w	#0x0700, sr
	/* Load handler, and nope out if nothing has been registered. */
	move.l	d0, -(sp)
	move.l	\symbol_name, d0
	beq	1f

	movem.l	d1/a0-a1, -(sp)
	move.l	d0, a0
	jsr	(a0)
	movem.l	(sp)+, d1/a0-a1
1:
	move.l	(sp)+, d0
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
