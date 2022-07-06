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

static void do_beep(uint16_t vol, uint16_t period)
{
	md_psg_pitch(0, period);
	md_psg_vol(0, vol);
	for (uint16_t i = 0; i < 12000; i++)
	{
		md_psg_vol(0, i / 1024);
	}
	md_psg_vol(0, 0xF);
}

static void beep_tone(uint16_t hi, uint16_t low)
{
	while (hi--)
	{
		do_beep(0, 0x180);
		do_beep(0xF, 0);
	}
	while (low--)
	{
		do_beep(0, 0x300);
		do_beep(0xF, 0);
	}
}

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
	beep_tone(1, 1);
	md_ioc_init();
	beep_tone(0, 8);
	md_sysc_vctrl_set_blank(1);
	md_sysc_vctrl_set_protection_reset(1);
	md_sysc_vctrl_set_md_color_compat(1);
	beep_tone(2, 1);
#endif

#ifndef MDK_TARGET_C2
	md_io_init();
#endif
	md_pal_init();

	beep_tone(3, 1);

	md_sys_ei();

	do_beep(0, 4*30);
	do_beep(0, 5*30);
	do_beep(0, 4*30);
	do_beep(0, 5*30);
	md_vdp_set_display_en(1);
}
