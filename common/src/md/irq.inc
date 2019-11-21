/* md-toolchain interrupt handlers
Michael Moffitt 2018

The hblank and serial interrupts (levels 4 and 2 respectively) are empty; they
should be expanded as-needed. */


/* This is used for vblank waiting. */
	.extern	g_vblank_wait
/* This is called to poll the controllers. */
	.extern	io_poll
/* This is called to fire off the DMA queue. */
	.extern	dma_q_process
/* IRQ vectors unused on the Megadrive */
_v_irq1:
_v_irq3:
_v_irq5:
_v_irq7:
	rte

/* /TH pin change, from controller port */
_v_irq2:
	rte

/* H-blank Interrupt */
_v_irq4:
	rte

/* V-blank Interrupt */
_v_irq6:
	/*
	This flag is cleared in vblank so that software may set it high,
	and then block on it transitioning low in order to synchronize.
	*/
	clr.w	g_vblank_wait
	jsr	io_poll
	jsr	dma_q_process
	rte
