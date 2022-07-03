#ifndef PLANE_H
#define PLANE_H

#include "md/megadrive.h"

static inline void plane_clear(VdpPlane plane);

static inline void plane_clear(VdpPlane plane)
{
	uint16_t plane_dest = vdp_get_plane_base(plane);
	uint16_t plane_size = vdp_get_reg(VDP_PLANESIZE);

	plane_size = (vdp_get_reg(VDP_PLANESIZE) & 0x03) * 64;
	plane_size *= (vdp_get_reg(VDP_PLANESIZE) & 0x30) * 4;

	sys_di();
	vdp_wait_dma();
	vdp_set_autoinc(1);
	dma_q_fill_vram(plane_dest, 0, plane_size, 1);
	vdp_wait_dma();
	sys_ei();
}

#endif
