// mdk top-level file
// 2018-2024 Michael Moffitt
// ===========================================================================
#include "md/megadrive.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/io.h"
#include "md/spr.h"
#include "md/sysc_vctrl.h"
#include <stdlib.h>

void megadrive_init(void)
{
	md_sys_di();
	md_sys_init();
	md_vdp_init();
	md_dma_init();
	md_spr_init(SPR_MODE_SIMPLE);
	md_dma_process();
	static const uint16_t scroll_default = 0;
	md_dma_transfer_vram(VRAM_HSCR_BASE_DEFAULT, &scroll_default, 1, 2);
	md_dma_transfer_vsram(0, &scroll_default, 1, 2);
	md_dma_process();

#ifdef MDK_TARGET_C2
	md_ioc_init();
	md_ioc_set_upd7759_reset(true);
	md_sysc_vctrl_set_blank(false);
	md_sysc_vctrl_set_protection_reset(true);
	md_sysc_vctrl_set_md_color_compat(true);
	md_ioc_set_udp7759_bank(0);
	md_ioc_set_upd7759_reset(false);
#else
	md_io_init();
#endif
	md_pal_init();
	md_vdp_set_display_en(true);

	md_sys_ei();
}

// Run after completing the logic in one game tick loop.
void megadrive_finish(void)
{
	const bool irq_en = md_sys_ei();
	md_spr_finish();
	md_pal_poll();
	md_vdp_wait_vblank();

	// C2-specific screen blanking.
#ifdef MDK_TARGET_C2
	md_sysc_vctrl_set_blank(true);
#endif  // MDK_TARGET_C2

#ifndef MDK_TARGET_C2
	md_io_poll();
#else
	md_ioc_poll();
#endif  // MDK_TARGET_C2
	md_dma_process();

	md_spr_start();

	// C2-specific screen blanking.
#ifdef MDK_TARGET_C2
	md_sysc_vctrl_set_blank(false);
#endif  // MDK_TARGET_C2
	if (!irq_en) md_sys_di();
}
