// mdk VDP control functions
// Michael Moffitt 2018-2024

#include "md/vdp.h"
#include "md/dma.h"
#include "md/sys.h"

#include <stdlib.h>

volatile uint16_t g_vblank_wait;
static void (*s_vbl_wait_func)(void) = NULL;

uint8_t g_md_vdp_regs[0x18];
uint16_t g_md_vdp_debug_regs[0x10];

static uint16_t s_plane_base[3];
static uint16_t s_sprite_base;
static uint16_t s_hscroll_base;

void md_vdp_init(void)
{
	md_vdp_set_hint_line(255);

	// H-int disabled
	md_vdp_set_reg(VDP_MODESET1, VDP_MODESET1_DEFAULT);

	// V-int enabled, display disabled, DMA enabled, 224-line mode, Mode 5
	md_vdp_set_reg(VDP_MODESET2, VDP_MODESET2_DEFAULT);

	// Plane scroll, TH ints disabled
	md_vdp_set_reg(VDP_MODESET3, VDP_MODESET3_DEFAULT);

	// H40 mode by default
	md_vdp_set_reg(VDP_MODESET4, VDP_MODESET4_DEFAULT);

	// Just in case...
	md_vdp_set_thint_en(false);
	md_vdp_set_hint_en(false);

	md_vdp_set_vscroll_mode(VDP_VSCROLL_PLANE);
	md_vdp_set_hscroll_mode(VDP_HSCROLL_PLANE);

	// BG color 0
	md_vdp_set_bg_color(0);

	// Clear VRAM
	md_vdp_vram_clear();

	// Standard 1-word auto inc
	md_vdp_set_autoinc(2);

	// Configure for 512 x 256px planes
	md_vdp_set_plane_size(VDP_PLANESIZE_64x32);

	md_vdp_set_window_top(0);
	md_vdp_set_window_left(0);
	// Set up VRAM base addresses
	md_vdp_set_plane_base(VDP_PLANE_A, VRAM_SCRA_BASE_DEFAULT);
	md_vdp_set_plane_base(VDP_PLANE_B, VRAM_SCRB_BASE_DEFAULT);
	md_vdp_set_plane_base(VDP_PLANE_WINDOW, VRAM_SCRW_BASE_DEFAULT);
	md_vdp_set_sprite_base(VRAM_SPR_BASE_DEFAULT);
	md_vdp_set_hscroll_base(VRAM_HSCR_BASE_DEFAULT);
}

void md_vdp_wait_vblank(void)
{
	g_vblank_wait = 1;
	while (g_vblank_wait)
	{
		if (s_vbl_wait_func) s_vbl_wait_func();
	}
}

void md_vdp_register_vblank_wait_callback(void (*function)(void))
{
	s_vbl_wait_func = function;
}

void md_vdp_set_plane_base(VdpPlane plane, uint16_t value)
{
	switch (plane)
	{
		default:
			return;
		case VDP_PLANE_A:
			md_vdp_set_reg(VDP_SCRABASE, (value >> 13) << 3);
			s_plane_base[VDP_PLANE_A] = value;
			break;
		case VDP_PLANE_B:
			md_vdp_set_reg(VDP_SCRBBASE, (value >> 13));
			s_plane_base[VDP_PLANE_B] = value;
			break;
		case VDP_PLANE_WINDOW:
			md_vdp_set_reg(VDP_SCRWBASE, (value >> 11) << 1);
			s_plane_base[VDP_PLANE_WINDOW] = value;
			break;
	}
}

void md_vdp_set_sprite_base(uint16_t value)
{
	s_sprite_base = value;
	md_vdp_set_reg(VDP_SPRBASE, (value  >>  9));
}

void md_vdp_set_hscroll_base(uint16_t value)
{
	s_hscroll_base = value;
	md_vdp_set_reg(VDP_HSCRBASE, (value >> 10));
}

uint16_t md_vdp_get_plane_base(VdpPlane plane)
{
	return s_plane_base[plane];
}

uint16_t md_vdp_get_sprite_base(void)
{
	return s_sprite_base;
}

uint16_t md_vdp_get_hscroll_base(void)
{
	return s_hscroll_base;
}
