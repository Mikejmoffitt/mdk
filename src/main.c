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

static uint16_t xs, ys;

void scroll_movement(void)
{
	uint16_t x_moved = 0;
	uint16_t y_moved = 0;
	if (io_pad_read(0) & BTN_RIGHT)
	{
		xs--;
		x_moved = 1;
	}
	else if (io_pad_read(0) & BTN_LEFT)
	{
		xs++;
		x_moved = 1;
	}
	if (io_pad_read(0) & BTN_DOWN)
	{
		ys--;
		y_moved = 1;
	}
	else if (io_pad_read(0) & BTN_UP)
	{
		ys++;
		y_moved = 1;
	}
	if (x_moved)
	{
		vdp_set_scroll_x(VDP_PLANE_A, xs);
	}
	if (y_moved)
	{
		vdp_set_scroll_y(VDP_PLANE_A, ys);
	}
}

void btn_draw(void)
{
	if (io_pad_read(0) & BTN_UP)
	{
		spr_put(64 - 48, 32 - 10, SPR_ATTR('U', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_DOWN)
	{
		spr_put(64 - 48, 32 + 10, SPR_ATTR('D', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_LEFT)
	{
		spr_put(64 - 60, 32, SPR_ATTR('L', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_RIGHT)
	{
		spr_put(64 - 36, 32, SPR_ATTR('R', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_START)
	{
		spr_put(64 + 12, 32 - 12, SPR_ATTR('S', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_A)
	{
		spr_put(64, 32, SPR_ATTR('A', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_B)
	{
		spr_put(64 + 12, 32, SPR_ATTR('B', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
	if (io_pad_read(0) & BTN_C)
	{
		spr_put(64 + 24, 32, SPR_ATTR('C', 0, 0, 0, 0), SPR_SIZE(1, 1));
	}
}

void main(void)
{
	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	// Clear the plane nametables
	sys_di();
	plane_clear(VDP_PLANE_A);
	plane_clear(VDP_PLANE_B);
	plane_clear(VDP_PLANE_WINDOW);
	sys_ei();

	// Set up text graphics at VRAM address 0x400 palette 0
	// This lines it up nicely with the actual ASCII values, which we will
	// use to draw letters with sprites.
	text_init(0x400, 0);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 14, 11, "Hello World");
	text_puts(VDP_PLANE_A, 5, 13, "(Scroll around with the d-pad)");

	while (1)
	{
		btn_draw();
		scroll_movement();

		// Set the background color to get an idea of how much CPU is utilized
		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
