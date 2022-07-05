/* mdk top-level file
Michael Moffitt */
#include "md/megadrive.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/io.h"
#include "md/spr.h"
#include "md/sysc_vctrl.h"

void megadrive_init(void)
{
	md_sys_di();
	md_sys_init();
	md_vdp_init();
	md_dma_init();
	md_io_init();
	md_spr_init();
	md_dma_fill_vram(0, 0, 32768, 2);
	md_dma_fill_vram(1, 0, 32768, 2);
	md_dma_process();
	md_vdp_set_display_en(1);
#ifdef MDK_TARGET_C2
	md_sysc_vctrl_set(MD_CVREG_PROTECTION_RESET | MD_CVREG_STANDARD_PAL_MODE);
#endif
	md_sys_ei();
}
