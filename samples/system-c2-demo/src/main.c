// md-toolchain example main.c
// Michael Moffitt 2018
//
// This main shows a simple "hello world" demo.

// megadrive.h is an umbrella for all headers in src/md. Specific modules like
// md/vdp.h do not need to be individually included. However, utility funcitons
// are not included, as they are not core support functions.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

static uint16_t x_scroll, y_scroll;

void main(void)
{
	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	// Set up text graphics at VRAM address 0x400 palette 0
	// This lines it up nicely with the actual ASCII values, which
	// a later demo will take advantage of.
	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);
	md_pal_upload_rgb333(0, res_font_pal_bin, 16);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 14, 11, "Hello System C2");

	// Center the screen at 0, 0.
	x_scroll = 0;
	y_scroll = 0;

	// Transfer our scroll coordinates to VRAM and VSRAM.
	md_dma_transfer_vram(VRAM_HSCR_BASE_DEFAULT, &x_scroll, 1, 2);
	md_dma_transfer_vsram(0, &y_scroll, 1, 2);

	while (1)
	{
		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
