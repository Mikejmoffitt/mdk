
#
# void md_dma_process_cmd(DmaCmd *cmd)
#
	.global	md_dma_process_cmd

.set	SPID, 4
.set	ARG_CMD, SPID  /* pointer to DmaCmd */
# Parameters in DmaCmd struct
.set	PRM_OP,     0  /* uint16_t */
.set	PRM_STRIDE, 2  /* uint8_t */
.set	PRM_SRC1,   3  /* uint8_t */
.set	PRM_SRC2,   4  /* uint8_t */
.set	PRM_SRC3,   5  /* uint8_t */
.set	PRM_LEN1,   6  /* uint8_t */
.set	PRM_LEN2,   7  /* uint8_t */
.set	PRM_CTRL32, 8  /* uint32_t, cache of ctrl longword for dest addr */

md_dma_process_cmd:
# A0 holds the DMA command.
	movea.l	ARG_CMD(sp), a0

# NOTE(2023-06-29): Repeatedly enabling and disabling the DMA bit does not
#                   appear to be necessary.
# Turn on DMA bit.
#	move.b	g_md_vdp_regs+VDP_MODESET2, d1  /* storage of original MODE2 */
#	move.w	#VDP_REGST_MODESET2, d0
#	move.b	d1, d0
#	bset	#4, d0  /* DMA enable */
#	move.w	d0, VDPPORT_CTRL

# Set auto inc.
	move.w	#VDP_REGST_AUTOINC, d0
	move.b	PRM_STRIDE(a0), d0
	move.w	d0, VDPPORT_CTRL
# Set length registers.
	move.w	#VDP_REGST_DMALEN1, d0
	move.b	PRM_LEN1(a0), d0
	move.w	d0, VDPPORT_CTRL
	move.w	#VDP_REGST_DMALEN2, d0
	move.b	PRM_LEN2(a0), d0
	move.w	d0, VDPPORT_CTRL
# If it's a fill operation, things are handled differently.
	tst.w	PRM_OP(a0)
	bmi	run_dma_fill
# Write remaining regs.
	move.w	#VDP_REGST_DMASRC1, d0
	move.b	PRM_SRC1(a0), d0
	move.w	d0, VDPPORT_CTRL
	move.w	#VDP_REGST_DMASRC2, d0
	move.b	PRM_SRC2(a0), d0
	move.w	d0, VDPPORT_CTRL
	move.w	#VDP_REGST_DMASRC3, d0
# The final source and destination addr writes set off the DMA.
	move.b	PRM_SRC3(a0), d0
	move.w	d0, VDPPORT_CTRL
	zbusreq
	move.l	PRM_CTRL32(a0), VDPPORT_CTRL
	zbusrel

# NOTE(2023-06-29): Repeatedly enabling and disabling the DMA bit does not
#                   appear to be necessary.
# Disable DMA.
#	move.w	#VDP_REGST_MODESET2, d0
#	move.b	d1, d0  /* Restore MODE2 from before. */
#	move.w	d0, VDPPORT_CTRL


# Mark the DMA command slot as inactive.
	clr.w	PRM_OP(a0)
	rts

run_dma_fill:
	move.w	#VDP_REGST_DMASRC3 | 0x80, VDPPORT_CTRL  /* Fill */
	move.l	PRM_CTRL32(a0), VDPPORT_CTRL
	move.b	PRM_SRC1(a0), d0
	move.b	d0, d1
	lsl.w	#8, d0
	move.b	d1, d0
	zbusreq
	move.w	d0, VDPPORT_DATA
	zbusrel
	/* Wait for DMA to complete */
dma_fill_vdp_wait:
	btst	#1,VDPPORT_CTRL+1
	bne	dma_fill_vdp_wait
	rts


#
# void md_dma_process(void)
#
	.global	md_dma_process

	.extern	g_md_sys_ints_enabled  /* bool */
md_dma_process_wip:
	dma_wait
# Disable ints, storing previous enablement status to restore later.
	move.b	g_md_sys_ints_enabled, d0
	move.w	d0, -(sp)
	clr.b	g_md_sys_ints_enabled
	andi.w	#0xF8FF, sr

# Handle the two sprite slots first.











# Restore int enablement and exit.
	move.w	(sp)+, d0
	move.b	d0, g_md_sys_ints_enabled
	beq	reenable_ints
	rts

reenable_ints:
	ori.w	#0x0700, sr
	rts
