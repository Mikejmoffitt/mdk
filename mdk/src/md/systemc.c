/* md-toolchain top-level file for System C / C2
Michael Moffitt */
#include "md/systemc.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/io_systemc.h"
#include "md/spr.h"

void systemc_init(void)
{
	sys_di();
	sys_init();
	vdp_init();
	dma_q_init();
	// TODO: Clear CRAM
	// TODO: Initialize sensible C/C2 register defaults
	spr_init();
	vdp_set_display_en(1);
	sys_ei();
}
