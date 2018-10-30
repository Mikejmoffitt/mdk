/* md-toolchain top-level file
Michael Moffitt */
#include "md/megadrive.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "md/io.h"
#include "md/spr.h"

void megadrive_init(void)
{
	sys_di();
	vdp_init();
	dma_q_set_budget(DMA_Q_BUDGET_AUTO);
	io_gamepad_en(0);
	io_gamepad_en(1);
	io_gamepad_en(2);
	spr_init();
	dma_wait();
	vdp_set_display_en(1);
	sys_ei();
}
