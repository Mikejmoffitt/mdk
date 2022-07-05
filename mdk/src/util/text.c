// mdk print utility functions
// Michael Moffitt 2018-2022
#include "util/text.h"

static uint16_t s_font_vram_pos;
static uint16_t s_font_pal_line;

void text_init(const unsigned char *font_chr, uint16_t font_len,
               uint16_t vram_pos,
               const unsigned char *font_pal, uint16_t pal_line)
{
	s_font_vram_pos = vram_pos;
	s_font_pal_line = pal_line & 0x3;
	if (font_chr)
	{
		md_dma_transfer_vram(s_font_vram_pos, font_chr, font_len/2, 2);
	}
	s_font_vram_pos /= 32;  // convert to tile number
	if (font_pal)
	{
		md_pal_upload(s_font_pal_line * 16, font_pal, 16);
	}
}

void text_puts(VdpPlane plane, uint16_t x, uint16_t y, const char *s)
{
	if (plane > VDP_PLANE_WINDOW)
	{
		return;
	}
	uint16_t dest_base = md_vdp_get_plane_base(plane);
	uint8_t plane_size = md_vdp_get_reg(VDP_PLANESIZE);
	uint16_t line_inc = 64;
	md_vdp_wait_dma();
	md_vdp_set_autoinc(2);
	switch (plane_size)
	{
		default:
			return;
		case VDP_PLANESIZE_32x32:
		case VDP_PLANESIZE_32x64:
		case VDP_PLANESIZE_32x128:
			line_inc = 64;
			break;
		case VDP_PLANESIZE_64x32:
		case VDP_PLANESIZE_64x64:
			line_inc = 128;
			break;
		case VDP_PLANESIZE_128x32:
			line_inc = 256;
			break;
	}

	dest_base += x * 2;
	dest_base += y * line_inc;

	VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);

	while (*s)
	{
		if (*s == '\n')
		{
			y++;
			dest_base = md_vdp_get_plane_base(plane);
			dest_base += x * 2;
			dest_base += y * line_inc;

			VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(dest_base);
		}
		else
		{
			VDPPORT_DATA = VDP_ATTR(((s_font_vram_pos + *s) - 0x20),
			                        0, 0, s_font_pal_line, 1);
		}
		s++;
	}
}

uint16_t text_get_vram_pos(void)
{
	return s_font_vram_pos;
}
