// mdk CD mode 1 sample
// Michael Moffitt 2022
#include "md/megadrive.h"

#include "util/text.h"
#include "util/plane.h"

#include "cdaudio.h"

#include "res.h"

void main(void)
{
	megadrive_init();
	text_init(res_font_gfx_bin, sizeof(res_font_gfx_bin), 0x400, res_font_pal_bin, 0);
	text_puts(VDP_PLANE_A, 1, 1, "Initializing CD...");
	megadrive_finish();

	const uint16_t init_result = cdaudio_init();
	if (init_result == 0)
	{
		text_puts(VDP_PLANE_A, 1, 1, "Init failed.       ");
		while (1) megadrive_finish();
	}

	text_puts(VDP_PLANE_A, 1, 1, "Waiting...        ");

	for (int i = 0; i < 60; i++)
	{
		megadrive_finish();
	}
	text_puts(VDP_PLANE_A, 1, 1, "Playing track 5...");

	cdaudio_play_loop(5);

	text_puts(VDP_PLANE_A, 1, 1, "Now playing.      ");
	// Wait forever.
	while (1)
	{
		megadrive_finish();
	}

}
