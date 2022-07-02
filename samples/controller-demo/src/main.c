// Michael Moffitt 2018-2022
//
// This main shows a simple "hello world" demo with some button interaction.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

void scroll_movement(void)
{
	static uint16_t xs, ys;

	if (md_io_pad_read(0) & BTN_RIGHT) xs++;
	else if (md_io_pad_read(0) & BTN_LEFT) xs--;
	if (md_io_pad_read(0) & BTN_DOWN) ys--;
	else if (md_io_pad_read(0) & BTN_UP) ys++;

	dma_q_transfer_vram(VRAM_HSCR_BASE_DEFAULT, &xs, 1, 2);
	dma_q_transfer_vsram(0, &ys, 1, 2);
}

void btn_draw(void)
{
	typedef struct ButtonMeta
	{
		uint16_t mask;
		char chara;
	} ButtonMeta;

	for (uint16_t j = 0; j < 2; j++)
	{
		const uint16_t buttons = md_io_pad_read(j);

		static const ButtonMeta button_meta[] =
		{
			{BTN_UP, 'U'},
			{BTN_DOWN, 'D'},
			{BTN_LEFT, 'L'},
			{BTN_RIGHT, 'R'},
			{BTN_A, 'A'},
			{BTN_B, 'B'},
			{BTN_C, 'C'},
			{BTN_X, 'X'},
			{BTN_Y, 'Y'},
			{BTN_Z, 'Z'},
			{BTN_START, 'S'},
			{BTN_MODE, 'M'},
		};

		uint16_t plot_x = 8;
		const uint16_t plot_y = 32 + (j * 8);
		spr_put(plot_x, plot_y, SPR_ATTR(j == 0 ? '1' : '2', 0, 0, 0, 0), SPR_SIZE(1, 1));
		plot_x += 16;
		if (buttons & MD_PAD_UNPLUGGED)
		{
			const char msg[] = "No Controller";
			for (uint16_t i = 0; i < sizeof(msg); i++)
			{
				spr_put(plot_x, plot_y, SPR_ATTR(msg[i], 0, 0, 0, 0), SPR_SIZE(1, 1));
				plot_x += 8;
			}
			continue;
		}
		for (uint16_t i = 0; i < sizeof(button_meta) / sizeof(button_meta[0]); i++)
		{
			const char chara = (buttons & button_meta[i].mask) ?
			                   button_meta[i].chara : '.';
			spr_put(plot_x, plot_y, SPR_ATTR(chara, 0, 0, 0, 0), SPR_SIZE(1, 1));
			plot_x += 8;
		}
	}
}

void main(void)
{
	// Get the megadrive ready to go! (See md/megadrive.h)
	megadrive_init();

	// Set up text graphics at VRAM address 0x400 palette 0
	// This lines it up nicely with the actual ASCII values, which
	// we will use to draw letters with sprites.
	text_init(res_gfx_font_bin, sizeof(res_gfx_font_bin), 0x400, res_pal_font_bin, 0);

	// Print a simple message in the center of plane A
	text_puts(VDP_PLANE_A, 14, 11, "Hello World");
	text_puts(VDP_PLANE_A, 5, 13, "(Scroll around with the d-pad)");

	while (1)
	{
		btn_draw();
		scroll_movement();

		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
