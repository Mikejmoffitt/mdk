#include "util/plane.h"

void plane_clear(VdpPlane plane)
{
	uint16_t plane_dest = md_vdp_get_plane_base(plane);
	uint16_t plane_size = md_vdp_get_reg(VDP_PLANESIZE);

	plane_size = (md_vdp_get_reg(VDP_PLANESIZE) & 0x03) * 64;
	plane_size *= (md_vdp_get_reg(VDP_PLANESIZE) & 0x30) * 4;

	md_dma_fill_vram(plane_dest, 0, plane_size, 1);
}
