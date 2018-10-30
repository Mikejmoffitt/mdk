/* md-toolchain VDP control functions
MIchael Moffitt 2018 */

#include "md/vdp.h"
#include "md/dma.h"
#include "md/sys.h"

volatile uint16_t g_vblank_wait;
uint8_t g_vdp_regvalues[0x18];
static uint16_t plane_base[3];
static uint16_t sprite_base;
static uint16_t hscroll_base;
static uint16_t hscroll_table[480];
static uint16_t vscroll_table[0x40];

void vdp_init(void)
{
	vdp_set_hint_line(255);

	// H-int disabled
	vdp_set_reg(VDP_MODESET1, VDP_MODESET1_BASE);

	// V-int enabled, display disabled, DMA enabled, 224-line mode, Mode 5
	vdp_set_reg(VDP_MODESET2, VDP_MODESET2_BASE |
	                          VDP_MODESET2_DMA_EN |
	                          VDP_MODESET2_VINT_EN);

	vdp_set_reg(VDP_MODESET3, VDP_MODESET3_BASE);

	// H40 mode by default
	vdp_set_reg(VDP_MODESET4, VDP_MODESET4_BASE | VDP_MODESET4_HMODE_H40);

	vdp_set_vscroll_mode(VDP_VSCROLL_PLANE);
	vdp_set_hscroll_mode(VDP_HSCROLL_PLANE);

	// BG color always 0/0
	vdp_set_bgcol(0 << 4);

	// Standard 1-word auto inc
	vdp_set_autoinc(2);

	// Configure for 512 x 256px planes
	vdp_set_plane_size(VDP_PLANESIZE_64x32);

	vdp_set_raster_height(224);
	vdp_set_raster_width(320);
	vdp_set_window_top(0);
	vdp_set_window_left(0);
	// Set up VRAM base addresses
	vdp_set_plane_base(VDP_PLANE_A, VRAM_SCRA_BASE);
	vdp_set_plane_base(VDP_PLANE_B, VRAM_SCRB_BASE);
	vdp_set_plane_base(VDP_PLANE_WINDOW, VRAM_SCRW_BASE);
	vdp_set_sprite_base(VRAM_SPR_BASE);
	vdp_set_hscroll_base(VRAM_HSCR_BASE);

	vdp_set_scroll_x(VDP_PLANE_A, 0);
	vdp_set_scroll_x(VDP_PLANE_B, 0);
	vdp_set_scroll_y(VDP_PLANE_A, 0);
	vdp_set_scroll_y(VDP_PLANE_B, 0);
}

void vdp_wait_vblank(void)
{
	g_vblank_wait = 1;
	while (g_vblank_wait)
	{

	}
}
static inline uint16_t hsram_len_calc(void)
{
	uint16_t len;
	uint8_t scroll_mode = vdp_get_reg(VDP_MODESET3) & 0x03;
	switch (scroll_mode)
	{
		default:
		case 0x00:
			len = 1;
			break;
		case 0x01:
			len = 16;
			break;
		case 0x02:
			len = vdp_get_raster_height() / 8;
			break;
		case 0x03:
			len = vdp_get_raster_height();
			break;
	}
	return len;
}

void vdp_set_scroll_x(VdpPlane plane, uint16_t value)
{
	if (plane > VDP_PLANE_B)
	{
		return;
	}
	uint16_t len = hsram_len_calc();
	for (uint16_t i = 0; i < len; i++)
	{
		hscroll_table[i*2 + plane] = 1024 - value;
	}
	vdp_hsram_upload((void *)hscroll_table);
}

void vdp_set_scroll_y(VdpPlane plane, uint16_t value)
{
	if (plane > VDP_PLANE_B)
	{
		return;
	}
	uint8_t scroll_mode = vdp_get_reg(VDP_MODESET3) & VDP_MODESET3_VSCROLL_CELL;
	// TODO: Replace this with a DMA fill?
	if (scroll_mode)
	{
		for (uint16_t i = 0; i < vdp_get_raster_width() / 16; i++)
		{
			vscroll_table[i*2 + plane] = value;
		}
	}
	else
	{
		vscroll_table[plane] = value;
	}
	vdp_vsram_upload((void *)vscroll_table);
}

void vdp_vsram_upload(void *vsram_data)
{
	uint16_t len = 1;
	if (vdp_get_reg(VDP_MODESET3) & VDP_MODESET3_VSCROLL_CELL)
	{
		len = vdp_get_raster_width() / 16;
	}
	dma_q_transfer_vsram(0, (void *)vsram_data, len * 2, 2);
}

void vdp_hsram_upload(void *hsram_data)
{
	uint16_t len = hsram_len_calc();
	dma_q_transfer_vram(hscroll_base, (void *)hsram_data, len * 2, 2);
}

void vdp_set_plane_base(VdpPlane plane, uint16_t value)
{
	switch (plane)
	{
		default:
			return;
		case VDP_PLANE_A:
			vdp_set_reg(VDP_SCRABASE, (value >> 13) << 3);
			plane_base[0] = value;
			break;
		case VDP_PLANE_B:
			vdp_set_reg(VDP_SCRBBASE, (value >> 13));
			plane_base[1] = value;
			break;
		case VDP_PLANE_WINDOW:
			vdp_set_reg(VDP_SCRWBASE, (value>> 11) << 1);
			plane_base[2] = value;
			break;
	}
}

void vdp_set_sprite_base(uint16_t value)
{
	sprite_base = value;
	vdp_set_reg(VDP_SPRBASE, (value  >>  9));
}

void vdp_set_hscroll_base(uint16_t value)
{
	hscroll_base = value;
	vdp_set_reg(VDP_HSCRBASE, (value >> 10));
}

uint16_t vdp_get_plane_base(VdpPlane plane)
{
	switch(plane)
	{
		default:
			return 0xFFFF;
		case VDP_PLANE_A:
			return plane_base[0];
		case VDP_PLANE_B:
			return plane_base[1];
		case VDP_PLANE_WINDOW:
			return plane_base[2];
	}
}

uint16_t vdp_get_sprite_base(void)
{
	return sprite_base;
}

uint16_t vdp_get_hscroll_base(void)
{
	return hscroll_base;
}

void vdp_set_raster_height(uint8_t height)
{
	if (height == 240 && sys_is_pal())
	{
		vdp_set_reg_bit(VDP_MODESET2, VDP_MODESET2_30H);
	}
	else
	{
		vdp_clear_reg_bit(VDP_MODESET2, VDP_MODESET2_30H);
	}
}

uint8_t vdp_get_raster_height(void)
{
	return (vdp_get_reg(VDP_MODESET2) & VDP_MODESET2_30H) ? 240 : 224;
}

void vdp_set_raster_width(uint16_t width)
{
	if (width == 320)
	{
		vdp_set_reg_bit(VDP_MODESET4, VDP_MODESET4_HMODE_H40);
	}
	else
	{
		vdp_clear_reg_bit(VDP_MODESET4, VDP_MODESET4_HMODE_H40);
	}
}

uint16_t vdp_get_raster_width(void)
{
	return (vdp_get_reg(VDP_MODESET4) & VDP_MODESET4_HMODE_H40) ? 320 : 256;
}
