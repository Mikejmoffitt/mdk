#include "bg_gradient.h"
#include "md/megadrive.h"

namespace
{
	static constexpr uint16_t kColorTable[] =
	{
		PALRGB(0, 0, 4),
		PALRGB(0, 0, 3),
		PALRGB(0, 0, 2),
		PALRGB(0, 0, 1),
		PALRGB(1, 0, 0),
		PALRGB(2, 0, 0),
		PALRGB(3, 0, 0),
		PALRGB(4, 0, 0),
		PALRGB(5, 0, 0),
		PALRGB(6, 0, 0),
		PALRGB(7, 1, 0),
		PALRGB(7, 2, 0),
		PALRGB(7, 3, 0),
		PALRGB(7, 4, 0),
		PALRGB(7, 5, 0),
		PALRGB(7, 6, 0),
		PALRGB(7, 7, 1),
		PALRGB(7, 7, 2),
		PALRGB(7, 7, 4),
		PALRGB(7, 7, 5),
		PALRGB(7, 7, 6),
		PALRGB(7, 7, 7),
		PALRGB(6, 7, 7),
		PALRGB(5, 7, 7),
		PALRGB(4, 7, 7),
		PALRGB(3, 7, 7),
		PALRGB(2, 7, 6),
		PALRGB(1, 7, 5),
		PALRGB(0, 7, 4),
		PALRGB(0, 7, 3),
		PALRGB(0, 6, 2),
		PALRGB(0, 5, 1),
		PALRGB(0, 4, 0),
		PALRGB(0, 3, 0),
		PALRGB(0, 2, 0),
		PALRGB(0, 1, 0),
		PALRGB(0, 0, 0),
	};

	static int s_color_index;
}  // namespace

void bg_gradient_on_hbl(void)
{
	if (s_color_index >= ARRAYSIZE(kColorTable)) s_color_index--;
	VDPPORT_CTRL32 = VDP_CTRL_CRAM_WRITE | VDP_CTRL_ADDR(0);
	// This ugly delay was tuned to move the CRAM write into HBlank.
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	VDPPORT_DATA = kColorTable[s_color_index++];

}

void bg_gradient_on_vbl(void)
{
	s_color_index = 0;
	bg_gradient_on_hbl();
}

void bg_gradient_init()
{
	s_color_index = 0;

	md_irq_register(MD_IRQ_VBLANK, bg_gradient_on_vbl);
	md_irq_register(MD_IRQ_HBLANK, bg_gradient_on_hbl);

	md_vdp_set_hint_line(5);
	md_vdp_set_hint_en(1);
}
