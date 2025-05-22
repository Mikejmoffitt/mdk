// Michael Moffitt 2018-2022
//
// This main shows a simple "hello world" demo with some button interaction.
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "res.h"

void draw_inputs(void)
{
	for (uint16_t j = 0; j < MDK_ARRAYSIZE(g_md_pad); j++)
	{
		uint16_t plot_x = 12 * 8;
		const uint16_t plot_y = (12 + j) * 8;

		// Player number indicator.
		md_spr_put(plot_x, plot_y, SPR_ATTR((j == 0 ? '1' : '2'), 0, 0, 0, 0),
		           SPR_SIZE(1, 1));
		plot_x += 16;

		// If the pad isn't plugged in, indicate as such and continue.
		if (g_md_pad[j] & MD_PAD_UNPLUGGED)
		{
			const char msg[] = "No Controller";
			for (uint16_t i = 0; i < sizeof(msg); i++)
			{
				md_spr_put(plot_x, plot_y, SPR_ATTR(msg[i], 0, 0, 0, 0), SPR_SIZE(1, 1));
				plot_x += 8;
			}
			continue;
		}

		// The button_meta struct is used to associate button masks (e.g.
		// BTN_A) with a char (e.g. 'a'). Each button is checked in a loop.
		// If the corresponding button is pressed, the character is printed.
		struct ButtonMeta
		{
			uint16_t mask;
			char chara;
		} button_meta[] =
		{
			{BTN_UP, 'u'},
			{BTN_DOWN, 'd'},
			{BTN_LEFT, 'l'},
			{BTN_RIGHT, 'r'},
			{BTN_A, 'a'},
			{BTN_B, 'b'},
			{BTN_C, 'c'},
			{BTN_X, 'x'},
			{BTN_Y, 'y'},
			{BTN_Z, 'z'},
			{BTN_START, 's'},
			{BTN_MODE, 'm'},
		};

		for (uint16_t i = 0; i < MDK_ARRAYSIZE(button_meta); i++)
		{
			char chara = (g_md_pad[j] & button_meta[i].mask) ?
			              button_meta[i].chara : '.';
			// Make letters uppercase if they are a positive edge.
			if (g_md_pad_pos[j] & button_meta[i].mask) chara &= ~0x20;
			md_spr_put(plot_x, plot_y, SPR_ATTR(chara, 0, 0, 0, 0), SPR_SIZE(1, 1));
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
	text_puts(VDP_PLANE_A, 10, 8, "Input Demonstration");

	while (1)
	{
		draw_inputs();

		megadrive_finish(); // Terminate the sprite list and wait for vblank
		// Controller polling and DMA queue process is handled in VBL ISR
	}

}
