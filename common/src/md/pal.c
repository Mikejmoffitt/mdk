/* md-toolchain palette support functions
Michael Moffitt 2018 */
#include "md/pal.h"
#include "md/dma.h"

void pal_set(uint8_t idx, uint16_t val)
{
	VDPPORT_CTRL32 = VDP_CTRL_CRAM_WRITE | VDP_CTRL_ADDR(idx << 1);
	VDPPORT_DATA = val;
}

void pal_upload(uint8_t dest, const void *source, uint8_t len)
{
	dma_q_transfer_cram(dest << 1, source, len, 2);
}
