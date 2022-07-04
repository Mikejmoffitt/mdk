/* mdk top-level file for System C / C2
Michael Moffitt */
#include "md/systemc.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/ioc.h"
#include "md/spr.h"

void systemc_init(void)
{
	md_sys_di();
	md_sys_init();
	md_vdp_init();
	md_dma_init();
	// TODO: Clear CRAM
	// TODO: Initialize sensible C/C2 register defaults
	md_spr_init();
	md_vdp_set_display_en(1);
	md_sys_ei();
}
