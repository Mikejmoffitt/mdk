// mdk C++ and DIRECT sprite mode example
// Michael Moffitt 2022

#include "md/megadrive.h"

#include "util/rand.h"
#include "bg_gradient.h"

#include "res.h"
#include "dot.h"

// Palette for the Dots (line 0)
static constexpr uint16_t kPalette[16] =
{
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 5),
	PALRGB(0, 1, 7),
	PALRGB(0, 2, 6),
	PALRGB(1, 3, 5),
	PALRGB(1, 4, 4),
	PALRGB(2, 5, 3),
	PALRGB(3, 5, 2),
	PALRGB(0, 0, 1),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
	PALRGB(0, 0, 0),
};

void main(void)
{
	megadrive_init();

	md_spr_init(SPR_MODE_DIRECT);
	srand(0);
	md_dma_transfer_vram(32, res_ball_bin, sizeof(res_ball_bin) / 2, 2);
	md_pal_upload(0, kPalette, 16);

	Dot dots[SPR_MAX];

	bg_gradient_init();

	while (1)
	{
		for (auto &dot : dots)
		{
			dot.Move();
			dot.Render();
		}
		megadrive_finish();
	}

}
