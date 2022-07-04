/* mdk VDP control functions
MIchael Moffitt 2018 */
#ifndef VDP_H
#define VDP_H

#include "md/mmio.h"
#include "md/sys.h"
#include <stdint.h>

// Tile / sprite attribute definition
#define VDP_ATTR(_tile, _hflip, _vflip, _pal, _prio) (((_tile) & 0x7FF) | \
                 ((_hflip) ? 0x800 : 0) | ((_vflip) ? 0x1000 : 0) | \
                 (((_pal) & 0x3) << 13) | ((_prio) ? 0x8000 : 0))

// Register names
#define VDP_MODESET1  0x00
#define VDP_MODESET2  0x01
#define VDP_SCRABASE  0x02
#define VDP_SCRWBASE  0x03
#define VDP_SCRBBASE  0x04
#define VDP_SPRBASE   0x05
#define VDP_128_SPCGA 0x06
#define VDP_BGCOL     0x07
#define VDP_UNUSED1   0x08
#define VDP_UNUSED2   0x09
#define VDP_HINTC     0x0A
#define VDP_MODESET3  0x0B
#define VDP_MODESET4  0x0C
#define VDP_HSCRBASE  0x0D
#define VDP_128_BPCGA 0x0E
#define VDP_AUTOINC   0x0F
#define VDP_PLANESIZE 0x10
#define VDP_WINHORI   0x11
#define VDP_WINVERT   0x12
#define VDP_DMALEN1   0x13
#define VDP_DMALEN2   0x14
#define VDP_DMASRC1   0x15
#define VDP_DMASRC2   0x16
#define VDP_DMASRC3   0x17

// Status flags
#define VDP_STATUS_PAL 0x0001
#define VDP_STATUS_DMA 0x0002
#define VDP_STATUS_HBLANK 0x0004
#define VDP_STATUS_VBLANK 0x0008
#define VDP_STATUS_ODD 0x0010
#define VDP_STATUS_SCOL 0x0020
#define VDP_STATUS_SOVR 0x0040
#define VDP_STATUS_VIP 0x0080
#define VDP_STATUS_FULL 0x0100
#define VDP_STATUS_EMPTY 0x0200

// Default VRAM layout, assuming 64x32-cell planes
// 0000-BFFF (1536 tiles) free
#define VRAM_SCRA_BASE_DEFAULT	0xC000
#define VRAM_SCRW_BASE_DEFAULT	0xD000
#define VRAM_SCRB_BASE_DEFAULT	0xE000
// F000-F7FF (64 tiles) free
#define VRAM_HSCR_BASE_DEFAULT	0xF800
#define VRAM_SPR_BASE_DEFAULT	0xFC00

// VRAM control words

#define VDP_CTRL_DMA_BIT     0x00000080

#define VDP_CTRL_VRAM_READ   0x00000000
#define VDP_CTRL_VRAM_WRITE  0x40000000
#define VDP_CTRL_VSRAM_READ  0x00000010
#define VDP_CTRL_VSRAM_WRITE 0x40000010
#define VDP_CTRL_CRAM_READ   0x00000020
#define VDP_CTRL_CRAM_WRITE  0xC0000000

#define VDP_DMA_SRC_FILL 0x80
#define VDP_DMA_SRC_COPY 0xC0

#define VDP_CTRL_ADDR(_addr) ((((uint32_t)(_addr) & 0x3FFF) << 16) | (((uint32_t)(_addr) & 0xC000) >> 14))


#define VDP_VRAMDEST(DEST)		(0x00004000 | ((uint32_t)(DEST) & 0x3FFF) | (((uint32_t)(DEST) & 0xC000) << 2))
#define VDP_VRAM32DEST(DEST)	(0x40000000 | (((uint32_t)(DEST) & 0x3FFF) << 16) | (((uint32_t)(DEST) & 0xC000) >> 14))
#define VDP_VSRAMDEST(DEST)		(0x00104000 | ((uint32_t)(DEST) & 0x3FFF) | (((uint32_t)(DEST) & 0xC000) << 2))
#define VDP_VSRAM32DEST(DEST)	(0x40000010 | (((uint32_t)(DEST) & 0x3FFF) << 16) | (((uint32_t)(DEST) & 0xC000) >> 14))
#define VDP_CRAMDEST(DEST)		(0x0000C000 | ((uint32_t)(DEST) & 0x3FFF) | (((uint32_t)(DEST) & 0xC000) << 2))

#define VDP_REG_WRITE(reg, val) do { VDPPORT_CTRL = 0x8000 | (reg << 8) | (val); } while(0)

#define VDP_MODESET1_BASE 0x04
#define VDP_MODESET1_LBLANK 0x20
#define VDP_MODESET1_HINT_EN 0x10
#define VDP_MODESET1_VC_ON_HS = 0x08
#define VDP_MODESET1_COLOR_LSB 0x04
#define VDP_MODESET1_HVCOUNT_STOP 0x02
#define VDP_MODESET1_OVERLAY 0x01

// Only mode 5 is valid on MD
#define VDP_MODESET2_BASE 0x04
// V30 is broken in NTSC mode
#define VDP_MODESET2_30H 0x08
#define VDP_MODESET2_DMA_EN 0x10
#define VDP_MODESET2_VINT_EN 0x20
#define VDP_MODESET2_DISP_EN 0x40
#define VDP_MODESET2_VRAM128 0x80


#define VDP_MODESET3_BASE 0x00
#define VDP_MODESET3_THINT_EN 0x10

#define VDP_MODESET3_VSCROLL_PLANE 0x00
#define VDP_MODESET3_VSCROLL_CELL 0x04

#define VDP_MODESET4_BASE 0x00
#define VDP_MODESET4_HMODE_H32			(0x00)
#define VDP_MODESET4_HMODE_H40			(0x81)
#define VDP_MODESET4_SHI_EN				(1 << 3)
#define VDP_MODESET4_INTERLACE_NONE		(0 << 1)

#define VDP_SET(regbase, mask, en) \
do \
{ \
	MD_SYS_BARRIER();\
	if(en) \
	{ \
		md_vdp_set_reg(regbase, md_vdp_get_reg(regbase) | (mask)); \
	} \
	else \
	{ \
		md_vdp_set_reg(regbase, md_vdp_get_reg(regbase) & ~(mask)); \
	} \
} while(0)

extern uint8_t g_md_vdp_regvalues[0x18];

typedef enum VdpHscrollMode
{
	VDP_HSCROLL_PLANE = 0x00,
	VDP_HSCROLL_UNDEF = 0x01,
	VDP_HSCROLL_CELL = 0x02,
	VDP_HSCROLL_LINE = 0x03
} VdpHscrollMode;

typedef enum VdpVscrollMode
{
	VDP_VSCROLL_PLANE = 0,
	VDP_VSCROLL_CELL = 0x04
} VdpVscrollMode;

typedef enum VdpHmode
{
	VDP_HMODE_H32 = 0x00,
	VDP_HMODE_H40 = 0x81
} VdpHmode;

typedef enum VdpVmode
{
	VDP_VMODE_V28 = 0x00,
	VDP_VMODE_V30 = 0x04
} VdpVmode;

typedef enum VdpPlaneSize
{
	VDP_PLANESIZE_32x32   = 0x00,
	VDP_PLANESIZE_64x32   = 0x01,
	VDP_PLANESIZE_UNDx32  = 0x02,
	VDP_PLANESIZE_128x32  = 0x03,

	VDP_PLANESIZE_32x64   = 0x10,
	VDP_PLANESIZE_64x64   = 0x11,
	VDP_PLANESIZE_UNDx64  = 0x12,
	VDP_PLANESIZE_128x64  = 0x13,

	VDP_PLANESIZE_32xUND  = 0x20,
	VDP_PLANESIZE_64xUND  = 0x21,
	VDP_PLANESIZE_UNDxUND = 0x22,
	VDP_PLANESIZE_128xUND = 0x23,

	VDP_PLANESIZE_32x128  = 0x30,
	VDP_PLANESIZE_64x128  = 0x31,
	VDP_PLANESIZE_UNDx128 = 0x32,
	VDP_PLANESIZE_128x128 = 0x33,

} VdpPlaneSize;

typedef enum VdpPlane
{
	VDP_PLANE_A = 0x00,
	VDP_PLANE_B = 0x01,
	VDP_PLANE_WINDOW = 0x02
} VdpPlane;

void md_vdp_init(void);

// Accessors
static inline void md_vdp_set_reg(uint8_t num, uint8_t val);
static inline void md_vdp_set_reg_bit(uint8_t num, uint8_t bit);
static inline void md_vdp_clear_reg_bit(uint8_t num, uint8_t bit);
static inline uint8_t md_vdp_get_reg(uint8_t num);
static inline uint16_t md_vdp_get_status(void);

// Interrupt config
static inline void md_vdp_set_hint_en(uint8_t enabled);
static inline void md_vdp_set_vint_en(uint8_t enabled);
static inline void md_vdp_set_thint_en(uint8_t enabled);
static inline void md_vdp_set_extint_en(uint8_t enabled);
static inline void md_vdp_set_hint_line(uint8_t line);
void md_vdp_wait_vblank(void);

// Address configuration

// Plane A, B:   Multiples of $2000
// Window plane: Multiples of $1000 in H40, $0800 in H32
void md_vdp_set_plane_base(VdpPlane plane, uint16_t value);
// Multiples of $0200
void md_vdp_set_sprite_base(uint16_t value);
// Multiples of $0400
void md_vdp_set_hscroll_base(uint16_t value);

uint16_t md_vdp_get_plane_base(VdpPlane plane);
uint16_t md_vdp_get_sprite_base(void);
uint16_t md_vdp_get_hscroll_base(void);

// Scroll plane config
static inline void md_vdp_set_hscroll_mode(VdpHscrollMode mode);
static inline void md_vdp_set_vscroll_mode(VdpVscrollMode mode);
static inline void md_vdp_set_plane_size(VdpPlaneSize size);
static inline VdpPlaneSize md_vdp_plane_size_from_cells(int16_t h_cells,
                                                     int16_t v_cells);
static inline void md_vdp_set_window_top(uint8_t height);
static inline void md_vdp_set_window_bottom(uint8_t height);
static inline void md_vdp_set_window_right(uint8_t width);
static inline void md_vdp_set_window_left(uint8_t width); // Buggy with horizontal scrolling!

// Blanking
static inline void md_vdp_set_display_en(uint8_t enabled);

// Raster config
static inline void md_vdp_set_hmode(VdpHmode mode);
static inline void md_vdp_set_vmode(VdpVmode mode);
static inline void md_vdp_set_bgcol(uint8_t idx);
static inline void md_vdp_set_shi(uint8_t enabled);
void md_vdp_set_raster_height(uint8_t height); // 224 or 240
void md_vdp_set_raster_width(uint16_t width); // 320 or 256
uint8_t md_vdp_get_raster_height(void);
uint16_t md_vdp_get_raster_width(void);

// Data transfer and DMA configuration
static inline void md_vdp_set_autoinc(uint8_t inc);
static inline void md_vdp_set_addr(uint16_t addr);
static inline void md_vdp_write(uint16_t value);
static inline uint16_t md_vdp_read(void);

static inline void md_vdp_poke(uint16_t addr, uint16_t value);
static inline uint16_t md_vdp_peek(uint16_t addr);

static inline void md_vdp_wait_dma(void);

// Accessors
static inline void md_vdp_set_reg(uint8_t num, uint8_t val)
{
	g_md_vdp_regvalues[num] = val;
	VDP_REG_WRITE(num, val);
}

static inline void md_vdp_set_reg_bit(uint8_t num, uint8_t bit)
{
	g_md_vdp_regvalues[num] |= bit;
	VDP_REG_WRITE(num, g_md_vdp_regvalues[num]);
}

static inline void md_vdp_clear_reg_bit(uint8_t num, uint8_t bit)
{
	g_md_vdp_regvalues[num] &= ~bit;
	VDP_REG_WRITE(num, g_md_vdp_regvalues[num]);
}

static inline uint8_t md_vdp_get_reg(uint8_t num)
{
	return g_md_vdp_regvalues[num];
}

static inline uint16_t md_vdp_get_status(void)
{
	return VDPPORT_CTRL;
}

// Interrupt config
static inline void md_vdp_set_hint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_HINT_EN, enabled);
}

static inline void md_vdp_set_vint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_VINT_EN, enabled);
}

static inline void md_vdp_set_thint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET3_THINT_EN, enabled);
}

static inline void md_vdp_set_extint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET3, VDP_MODESET3_THINT_EN, enabled);
}

static inline void md_vdp_set_hint_line(uint8_t line)
{
	md_vdp_set_reg(VDP_HINTC, line);
}

// Scroll plane config
static inline void md_vdp_set_hscroll_mode(VdpHscrollMode mode)
{
	md_vdp_set_reg(VDP_MODESET3, (md_vdp_get_reg(VDP_MODESET3) & 0xFC) | mode);
}

static inline void md_vdp_set_vscroll_mode(VdpVscrollMode mode)
{
	VDP_SET(VDP_MODESET3, VDP_MODESET3_VSCROLL_CELL, mode);
}

static inline void md_vdp_set_plane_size(VdpPlaneSize size)
{
	md_vdp_set_reg(VDP_PLANESIZE, size);
}

static inline VdpPlaneSize md_vdp_plane_size_from_cells(int16_t h_cells,
                                                     int16_t v_cells)
{
	return ((h_cells / 32) - 1) | (((v_cells / 32) - 1) << 4);
}

static inline void md_vdp_set_window_top(uint8_t height)
{
	md_vdp_set_reg(VDP_WINVERT, height & 0x1F);
}

static inline void md_vdp_set_window_bottom(uint8_t height)
{
	md_vdp_set_reg(VDP_WINVERT, 0x80 | ((31-height) & 0x1F));
}

static inline void md_vdp_set_window_right(uint8_t width)
{
	md_vdp_set_reg(VDP_WINHORI, 0x80 | ((31-width) & 0x1F));
}

static inline void md_vdp_set_window_left(uint8_t width)
{
	md_vdp_set_reg(VDP_WINHORI, width & 0x1F);
}

// Blanking
static inline void md_vdp_set_display_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_DISP_EN, enabled);
}

// Raster config
static inline void md_vdp_set_hmode(VdpHmode mode)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_HMODE_H40, mode);
}

static inline void md_vdp_set_vmode(VdpVmode mode)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_30H, mode);
}

static inline void md_vdp_set_bgcol(uint8_t idx)
{
	md_vdp_set_reg(VDP_BGCOL, idx);
}

static inline void md_vdp_set_shi(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_SHI_EN, enabled);
}

// Data transfer and DMA configuration
static inline void md_vdp_set_autoinc(uint8_t inc)
{
	md_vdp_set_reg(VDP_AUTOINC, inc);
}

static inline void md_vdp_set_addr(uint16_t addr)
{
	VDPPORT_CTRL32 = VDP_CTRL_VRAM_WRITE | VDP_CTRL_ADDR(addr);
}

static inline void md_vdp_write(uint16_t value)
{
	VDPPORT_DATA = value;
}

static inline uint16_t md_vdp_read(void)
{
	return VDPPORT_DATA;
}

static inline void md_vdp_wait_dma(void)
{
	while(md_vdp_get_status() & VDP_STATUS_DMA)
	{
		__asm__ volatile ("\tnop\n");
	}
}

static inline void md_vdp_poke(uint16_t addr, uint16_t value)
{
	md_vdp_set_addr(addr);
	md_vdp_write(value);
}

static inline uint16_t md_vdp_peek(uint16_t addr)
{
	md_vdp_set_addr(addr);
	return md_vdp_read();
}

// HV Counter
static inline uint16_t md_vdp_get_hv_count(void)
{
	return VDPPORT_HVCOUNT;
}

static inline uint8_t md_vdp_get_h_count(void)
{
	return VDPPORT_HVCOUNT & 0x00FF;
}

static inline uint8_t md_vdp_get_v_count(void)
{
	return (VDPPORT_HVCOUNT) >> 8;
}


#endif // VDP_H
