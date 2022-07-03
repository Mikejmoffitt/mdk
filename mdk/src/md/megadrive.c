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
	sys_init();
	vdp_init();
	dma_q_init();
	md_io_init();
	spr_init();
	dma_q_fill_vram(0, 0, 32768, 2);
	dma_q_fill_vram(1, 0, 32768, 2);
	dma_q_process();
	vdp_set_display_en(1);
	sys_ei();
}
