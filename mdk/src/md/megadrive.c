/* mdk top-level file
Michael Moffitt */
#include "md/megadrive.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/io.h"
#include "md/spr.h"
#include "md/sysc_vctrl.h"
#include "md/tmss.h"
#include <stdlib.h>

void megadrive_init(void)
{
	md_sys_di();
	md_sys_init();
	md_vdp_init();
	md_dma_init();
	md_spr_init();
	md_dma_fill_vram(0, 0, 32768, 2);
	md_dma_fill_vram(1, 0, 32768, 2);
	md_dma_process();

#ifdef MDK_TARGET_C2
	md_ioc_init();
	md_ioc_set_upd7759_reset(1);
	md_sysc_vctrl_set_blank(1);
	md_sysc_vctrl_set_protection_reset(1);
	md_sysc_vctrl_set_md_color_compat(1);
	md_ioc_set_udp7759_bank(0);
	md_ioc_set_upd7759_reset(0);
#else
	md_io_init();
#endif
	md_pal_init();
	md_sys_ei();
	md_vdp_set_display_en(1);
}
