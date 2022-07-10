// MDK SEGA 315-5313 / YM7101 support (VDP)
// Michael Moffitt 2018-2022
//
// The VDP (Video Display Processor) is the most complicated part of the Sega
// Mega Drive, Genesis, and System C/C2. It is responsible for generating the
// graphics, producing interrupts, and performing DMA data transfers.
//
// The VDP is controlled via a series of registers. Functions for just about
// every "useful" register have been defined, and generic register accessors
// exist for all of them. As registers can not be read back from the VDP, a
// small cache of register data exists in RAM to facilitate modifying single
// register bits.
//
// vdp_init() sets up default values, and is called by megadrive_init().
//
// The Mega Drive presents 64KiB of Video RAM, attached directly to the VDP.
// This memory is not mapped into the main CPU address space; we cannot just
// write data to it. All VRAM access is therefore done through the VDP. For
// any mass amount of data (graphics tiles, plane tables, sprite tables) data
// is generally transferred by DMA. `dma.h` has been created to encompass and
// facilitate the use of the VDP's DMA features, as it is large and complex
// enough to warrant it.

#ifndef MD_VDP_H
#define MD_VDP_H

#include "md/mmio.h"
#include "md/sys.h"
#include <stdint.h>

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

// Tile / sprite attribute definition macro.
#define VDP_ATTR(_tile, _hflip, _vflip, _pal, _prio) (((_tile) & 0x7FF) | \
                 ((_hflip) ? 0x800 : 0) | ((_vflip) ? 0x1000 : 0) | \
                 (((_pal) & 0x3) << 13) | ((_prio) ? 0x8000 : 0))

// Macro to form command to set an address via the control port.
#define VDP_CTRL_ADDR(_addr) ((((uint32_t)(_addr) & 0x3FFF) << 16) | (((uint32_t)(_addr) & 0xC000) >> 14))

// -----------------------------------------------------------------------------
// Default VRAM layout - Optimized for 64x32 cell planes.
// These are used during init, but you are not obligated to stick to them.
// -----------------------------------------------------------------------------

// 0000-BFFF (1536 tiles) free
#define VRAM_SCRA_BASE_DEFAULT  0xC000
#define VRAM_SCRW_BASE_DEFAULT  0xD000
#define VRAM_SCRB_BASE_DEFAULT  0xE000
// F000-F7FF (64 tiles) free
#define VRAM_HSCR_BASE_DEFAULT  0xF800
#define VRAM_SPR_BASE_DEFAULT   0xFC00

// =============================================================================
// Enums and defines
// =============================================================================

// -----------------------------------------------------------------------------
// Mode Set Register bit values
// -----------------------------------------------------------------------------
// These mode set register bits have somewhat obtuse names, and the impact they
// have on a program is not always obvious. Furthermore, the use cases for them
// may not be clear. This section exists mostly to document them, but the actual
// use cases for them are mostly expressed via various accessor functions below,
// which are given more descriptive names.
//
// Thanks to Jorge Nuno, Charles MacDonald, Nemesis, Tiido Priimagi, and others
// for clarification on the functionality of a lot of these.

// Register $00 - Mode Set Register 1
// v... .... SMSVL - Columns 24-31 (but not 32-39) forced to a scroll of 0.
// .h.. .... SMSHL - SMS Legacy, unused or unknown effect.
// ..l. .... LCB   - Leftmost column blank. Covers leftmost 8-pixel column.
// ...I .... IE1   - Horizontal interrupt enable. Will trigger when set.
// .... e... VC0   - Outputs bit 0 of V. counter on HSync pin. Set to 0!
// .... .f.. FCE   - Full color enable. Enables full palette bit depth.
// .... ..s. M3    - HV counter latch enable. When set, HV counter is stopped.
// .... ...o OVER  - Overlay mode. Enables locking to ext. sync via csync pin.

#define VDP_MODESET1_SMSVL 0x80
#define VDP_MODESET1_SMSHL 0x40
#define VDP_MODESET1_LCB   0x20
#define VDP_MODESET1_IE1   0x10
#define VDP_MODESET1_VC0   0x08
#define VDP_MODESET1_FCE   0x04
#define VDP_MODESET1_M3    0x02
#define VDP_MODESET1_OVER  0x01

#define VDP_MODESET1_DEFAULT (VDP_MODESET1_FCE)

// Register $01 - Mode Set Register 2
// v... .... VR128 - VRAM 128K mode. Enables bus for second set of VRAM.
// .d.. .... DISP  - Enables display. Shows the background color when set to 0.
// ..I. .... IE0   - Vertical interrupt enable.
// ...d .... M1    - Enables DMA functionality.
// .... h... M2    - Sets vertical size to 30 cells (PAL only).
// .... .m.. M5    - Enables MD display mode. When cleared, VDP is in SMS mode.
// .... ..s. SMSSZ - Obsolete SMS sprite size selection.
// .... ...M SMSMG - Obsolete SMS sprite mag. Makes scroll offset sync pulse.

#define VDP_MODESET2_VR128 0x80
#define VDP_MODESET2_DISP  0x40
#define VDP_MODESET2_IE0   0x20
#define VDP_MODESET2_M1    0x10
#define VDP_MODESET2_M2    0x08
#define VDP_MODESET2_M5    0x04
#define VDP_MODESET2_SMSSZ 0x02
#define VDP_MODESET2_SMSMG 0x01

#define VDP_MODESET2_DEFAULT (VDP_MODESET2_M5 | \
                              VDP_MODESET2_M1 | \
                              VDP_MODESET2_IE0)

// Register $0B - Mode Set Register 3
// a... .... ADMUX - On CBUS, outputs color code when set, else CPU address.
// .c.. .... DRAMS - Enables work DRAM signals when set. Set to 0.
// ..xx .... UNK   - Unknown meaning or effect.
// .... I... IE2   - Enables external interrupts (e.g. controller TH pin).
// .... .v.. VCELL - Vertical scroll mode. See VdpVmode enum.
// .... ..h. HS1   - Horizontal scroll mode (bit 1). See VdpHmode enum.
// .... ...l HS0   - Horizontal scroll mode (bit 0).

#define VDP_MODESET3_ADMUX 0x80
#define VDP_MODESET3_DRAMS 0x40
#define VDP_MODESET3_IE2   0x08
#define VDP_MODESET3_VCELL 0x04
#define VDP_MODESET3_HS1   0x02
#define VDP_MODESET3_HS0   0x01

#define VDP_MODESET3_DEFAULT 0

// Register $0C - Mode Set Register 4
// r... .... RS0   - Select external dot clock (EDCLK). Used for H40 on MD.
// .s.. .... VSCLK - Outputs pixel clock on VSync pin. Used by 32x and C/C2.
// ..h. .... HSCIN - Hsync pin becomes an input. Used by 32x.
// ...c .... SPAEN - Enable sprite/plane indicator pin as output. Used by C/C2.
// .... S... SHI   - Enable shadow/highlight mode.
// .... .L.. LSM1  - Interlace mode (bit 1). See VdpInterlaceMode enum.
// .... ..l. LSM0  - Interlace mode (bit 0).
// .... ...R RS1   - Selects horizontal cell mode and dot clock divisor.

#define VDP_MODESET4_RS1   0x80
#define VDP_MODESET4_VSCLK 0x40
#define VDP_MODESET4_HSCIN 0x20
#define VDP_MODESET4_SPAEN 0x10
#define VDP_MODESET4_SHI   0x08
#define VDP_MODESET4_LSM1  0x04
#define VDP_MODESET4_LSM0  0x02
#define VDP_MODESET4_RS0   0x01

#ifndef MDK_TARGET_C2
// Megadrive requires RS1 to be set to accept EDCLK so that the horizontal scan
// rate is the correct ~15.7KHz.
#define VDP_MODESET4_DEFAULT (VDP_MODESET4_RS0 | \
                              VDP_MODESET4_RS1)
#else
// System C/C2 do not have EDCLK, so the VDP's internal H40 clock is used. This
// produces a horizontal scan rate of closer to 16.0KHz.
// The SPA/B pin and VSCLK pins need to be used for the external color DAC to
// work correctly.
#define VDP_MODESET4_DEFAULT (VDP_MODESET4_RS0 | \
                              VDP_MODESET4_SPAEN | \
                              VDP_MODESET4_VSCLK)
#endif

// Status flags
#define VDP_STATUS_PAL    0x0001
#define VDP_STATUS_DMA    0x0002
#define VDP_STATUS_HBLANK 0x0004
#define VDP_STATUS_VBLANK 0x0008
#define VDP_STATUS_ODD    0x0010
#define VDP_STATUS_SCOL   0x0020
#define VDP_STATUS_SOVR   0x0040
#define VDP_STATUS_VIP    0x0080
#define VDP_STATUS_FULL   0x0100
#define VDP_STATUS_EMPTY  0x0200

// -----------------------------------------------------------------------------
// Register names
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// VRAM control words
// -----------------------------------------------------------------------------
#define VDP_CTRL_DMA_BIT     0x00000080
#define VDP_CTRL_VRAM_READ   0x00000000
#define VDP_CTRL_VRAM_WRITE  0x40000000
#define VDP_CTRL_VSRAM_READ  0x00000010
#define VDP_CTRL_VSRAM_WRITE 0x40000010
#define VDP_CTRL_CRAM_READ   0x00000020
#define VDP_CTRL_CRAM_WRITE  0xC0000000

// DMA operations
#define VDP_DMA_SRC_FILL 0x80
#define VDP_DMA_SRC_COPY 0xC0

// The register cache.
extern uint8_t g_md_vdp_regvalues[0x18];

// Enums for register access functions.
typedef enum VdpHscrollMode
{
	VDP_HSCROLL_PLANE = 0,
	VDP_HSCROLL_UNDEF = VDP_MODESET3_HS0,
	VDP_HSCROLL_CELL = VDP_MODESET3_HS1,
	VDP_HSCROLL_LINE = VDP_MODESET3_HS0 | VDP_MODESET3_HS1
} VdpHscrollMode;

typedef enum VdpVscrollMode
{
	VDP_VSCROLL_PLANE,
	VDP_VSCROLL_CELL
} VdpVscrollMode;

typedef enum VdpHmode
{
	VDP_HMODE_H40,
	VDP_HMODE_H32
} VdpHmode;

typedef enum VdpVmode
{
	VDP_VMODE_V28,
	VDP_VMODE_V30
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

typedef enum VdpInterlaceMode
{
	VDP_INTERLACE_NONE,
	VDP_INTERLACE_NORMAL,
	VDP_INTERLACE_DOUBLE
} VdpInterlaceMode;

// =============================================================================
// Functions and accessors
// =============================================================================
void md_vdp_init(void);

// -----------------------------------------------------------------------------
// Generic Accessors
// -----------------------------------------------------------------------------

// Sets a full register value, and updates cache accordingly.
static inline void md_vdp_set_reg(uint8_t num, uint8_t val);

// Returns the last register value from the cache.
static inline uint8_t md_vdp_get_reg(uint8_t num);

// Returns the VDP status register contents.
static inline uint16_t md_vdp_get_status(void);

// -----------------------------------------------------------------------------
// Interrupt config
// -----------------------------------------------------------------------------

// The VDP's interrupt sources. Handlers may be registered for these interrupts
// in `irq.h` as normal callback functions.
// Even if these sources are enabled, interrupts may be masked with sys_di().

// Enables an IRQ that fires at the start of vertical blank, once per frame.
static inline void md_vdp_set_vint_en(uint8_t enabled);
// Enables an IRQ that fires upon transition of the controller's TH pin.
static inline void md_vdp_set_thint_en(uint8_t enabled);
// Enables an IRQ that fires at the horizontal blank of a scanline.
static inline void md_vdp_set_hint_en(uint8_t enabled);
// Set the number of scanlines between horizontal interrupts.
static inline void md_vdp_set_hint_line(uint8_t line);

// Upon calling this, the CPU will wait until the vertical interrupt is fired,
// at which point this routine will return. This is dependent on the vertical
// interrupt being enabled, so do not call this if those have been disabled!
void md_vdp_wait_vblank(void);

// -----------------------------------------------------------------------------
// Base Address configuration
// -----------------------------------------------------------------------------

// Tell the VDP where various tables are located in VRAM.

// Plane A, B:   Multiples of $2000
// Window plane: Multiples of $1000 in H40, $0800 in H32
void md_vdp_set_plane_base(VdpPlane plane, uint16_t value);
// Sprite base: Multiples of $0200
void md_vdp_set_sprite_base(uint16_t value);
// H scroll table base: Multiples of $0400
void md_vdp_set_hscroll_base(uint16_t value);

uint16_t md_vdp_get_plane_base(VdpPlane plane);
uint16_t md_vdp_get_sprite_base(void);
uint16_t md_vdp_get_hscroll_base(void);

// -----------------------------------------------------------------------------
// Scroll planes
// -----------------------------------------------------------------------------

// Sets the size of the scroll planes (they are set together).
// Not all values are valid and respected by the VDP, so please use a value
// from the VdpPlaneSize enum only.
static inline void md_vdp_set_plane_size(VdpPlaneSize size);

// Sets the horizontal and vertical scroll modes.
static inline void md_vdp_set_hscroll_mode(VdpHscrollMode mode);
static inline void md_vdp_set_vscroll_mode(VdpVscrollMode mode);

// Get the current plane dimensions in cells (pixels / 8).
static inline uint16_t md_vdp_get_plane_width(void);
static inline uint16_t md_vdp_get_plane_height(void);

// Enable the window plane, and set its size (in cells) from a screen edge.
// Call any of these with 0 to disable the window plane.
static inline void md_vdp_set_window_top(uint8_t height);
static inline void md_vdp_set_window_bottom(uint8_t height);
static inline void md_vdp_set_window_right(uint8_t width);
static inline void md_vdp_set_window_left(uint8_t width); // Buggy with H scroll

// -----------------------------------------------------------------------------
// Raster config
// -----------------------------------------------------------------------------

// Sets the display output on or off. If turned off, the VDP will just output
// the color pointed to by the background color index (md_vdp_set_bg_color()).
static inline void md_vdp_set_display_en(uint8_t enabled);

// Set the "background color" output in the display's blanking region. This
// refers to an index in color RAM, so the color seen depends on the palette.
static inline void md_vdp_set_bg_color(uint8_t idx);

// Set the display resolution. VDP_VMODE_V30 is only available for PAL/50Hz.)
static inline void md_vdp_set_hmode(VdpHmode mode);
static inline void md_vdp_set_vmode(VdpVmode mode);

// Get the size of the display in pixels. Dependent on H and V modes set above.
static uint16_t md_vdp_get_raster_height(void);
static uint16_t md_vdp_get_raster_width(void);

// Enable shadow/highlight mode. In this mode, the priority bit for the scroll
// darkens the cell when cleared. Sprites will be shaded by the background cell
// if their priority bit is not set. Furthermore, palette entries $3E and $3F
// output a special "highlight" and "shadow" color respectively when used by
// sprites.
static inline void md_vdp_set_shadow_highlight(uint8_t enabled);

// Blanks the leftmost eight columns of the screen to the background color.
static inline void md_vdp_set_left_column_blank(uint8_t enabled);

// Enable or disable one of the interlaced modes.
static inline void md_vdp_set_interlace_mode(VdpInterlaceMode mode);

// Output a dot clock signal on the VDP's vertical sync pin. This is used as a
// pixel data latch on System 18 and System C/C2.
static inline void md_vdp_set_vs_clk_output(uint8_t enabled);

// Enable output of the sprite/background pixel indicator pin on the VDP. This
// is used for color RAM addressing on System 18 and System C/C2.
static inline void md_vdp_set_spa_output(uint8_t enabled);

// When set, the VDP will output the pixel color index on the color bus pins.
// When cleared, the VDP allows the CPU address to drive those pins.
// System C/C2 use this to write to Color RAM. It should be set during active
// display, and only cleared when external Color RAM is being written to.
static inline void md_vdp_set_cbus_cpu_mux(uint8_t enabled);

// Turns the VDP's horizontal sync pin into an input. This will cause a loss
// of sync on a standard Mega Drive / Genesis.
static inline void md_vdp_set_hs_input(uint8_t enabled);

// Set the (obsolete) vertical and horizontal lock registers, which are
// legacy registers carried over from the SMS.
static inline void md_vdp_set_sms_vl(uint8_t enabled);
static inline void md_vdp_set_sms_hl(uint8_t enabled);

// -----------------------------------------------------------------------------
// H/V Counter
// -----------------------------------------------------------------------------

// Get the raster popsition from the HV counter.
static inline uint16_t md_vdp_get_hv_count(void);

// Get the horizontal component of the HV counter.
static inline uint8_t md_vdp_get_h_count(void);

// Get the vertical component of the HV counter.
static inline uint8_t md_vdp_get_v_count(void);

// Latch the HV counter value, causing it to stop. This is used with light-guns.
static inline void md_vd_set_hv_count_latch(uint8_t latch);

// -----------------------------------------------------------------------------
// Data transfer and DMA configuration
// -----------------------------------------------------------------------------

// Set the VDP's address pointer.
static inline void md_vdp_set_addr(uint16_t addr);

// Write data to the VDP. This data generally ends up at the address pointed to
// by md_vdp_set_addr().
static inline void md_vdp_write(uint16_t value);

// Read from the VDP.
static inline uint16_t md_vdp_read(void);

// Set the amount added to the VDP's address register after data is written.
// A typical value of 2 is used, so that words (two bytes) may be written in
// succession.
static inline void md_vdp_set_autoinc(uint8_t inc);

// Combination of md_vdp_set_addr() and md_vdp_write().
static inline void md_vdp_poke(uint16_t addr, uint16_t value);

// Combination of md_vdp_set_addr() and md_vdp_read().
static inline uint16_t md_vdp_peek(uint16_t addr);

// Enables the DMA unit. Generally used by `dma.c`.
static inline void md_vdp_set_dma_en(uint8_t enabled);

// Blocks until the VDP status register indicates a DMA is no longer in
// progress. Used by `dma.c`.
static inline void md_vdp_wait_dma(void);





// =============================================================================
// Implementations
// =============================================================================

// -----------------------------------------------------------------------------
// Internal use macros, for interacting with VDP registers.
// -----------------------------------------------------------------------------
// Macro to set a register and update the cached value.
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

// Macro to write a value to a register directly.
// Unless you have a good reason to do otherwise, use the md_vdp_set functions.
// This bypasses the register cache that is used elsewhere, so this is not
// recommended for regular use.
#define VDP_REG_WRITE(reg, val) do { VDPPORT_CTRL = 0x8000 | (reg << 8) | (val); } while(0)

// Accessors
static inline void md_vdp_set_reg(uint8_t num, uint8_t val)
{
	g_md_vdp_regvalues[num] = val;
	VDP_REG_WRITE(num, val);
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
	VDP_SET(VDP_MODESET1, VDP_MODESET1_IE1, enabled);
}

static inline void md_vdp_set_vint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_IE0, enabled);
}

static inline void md_vdp_set_thint_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET3, VDP_MODESET3_IE2, enabled);
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
	VDP_SET(VDP_MODESET3, VDP_MODESET3_VCELL, mode == VDP_VSCROLL_CELL);
}

static inline void md_vdp_set_plane_size(VdpPlaneSize size)
{
	md_vdp_set_reg(VDP_PLANESIZE, size);
}

static inline uint16_t md_vdp_get_plane_width(void)
{
	switch (md_vdp_get_reg(VDP_PLANESIZE))
	{
		default:
			return 0;
		case VDP_PLANESIZE_32x32:
		case VDP_PLANESIZE_32x64:
		case VDP_PLANESIZE_32x128:
			return 32;
		case VDP_PLANESIZE_64x32:
		case VDP_PLANESIZE_64x64:
		case VDP_PLANESIZE_64x128:
			return 64;
		case VDP_PLANESIZE_128x32:
		case VDP_PLANESIZE_128x64:
		case VDP_PLANESIZE_128x128:
			return 128;
	}
}

static inline uint16_t md_vdp_get_plane_height(void)
{
	switch (md_vdp_get_reg(VDP_PLANESIZE))
	{
		default:
			return 0;
		case VDP_PLANESIZE_32x32:
		case VDP_PLANESIZE_64x32:
		case VDP_PLANESIZE_128x32:
			return 32;
		case VDP_PLANESIZE_32x64:
		case VDP_PLANESIZE_64x64:
		case VDP_PLANESIZE_128x64:
			return 64;
		case VDP_PLANESIZE_32x128:
		case VDP_PLANESIZE_64x128:
		case VDP_PLANESIZE_128x128:
			return 128;
	}
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
	VDP_SET(VDP_MODESET2, VDP_MODESET2_DISP, enabled);
}

// Raster config
static inline void md_vdp_set_hmode(VdpHmode mode)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_RS1, mode == VDP_HMODE_H40);
#ifndef MDK_TARGET_C2
	VDP_SET(VDP_MODESET4, VDP_MODESET4_RS0, mode == VDP_HMODE_H40);
#else
	// Supposedly, System C/C2 cannot do H32 mode.
	VDP_SET(VDP_MODESET4, VDP_MODESET4_RS0, 0);
#endif
}

static inline void md_vdp_set_vmode(VdpVmode mode)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_M2, mode == VDP_VMODE_V30);
}

static inline void md_vdp_set_bg_color(uint8_t idx)
{
	md_vdp_set_reg(VDP_BGCOL, idx);
}

static inline void md_vdp_set_shadow_highlight(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_SHI, enabled);
}

static inline void md_vdp_set_left_column_blank(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET1_LCB, enabled);
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

static inline void md_vdp_set_dma_en(uint8_t enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_M1, enabled);
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

static inline void md_vd_set_hv_count_latch(uint8_t latch)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_M3, latch);
}

static uint16_t md_vdp_get_raster_height(void)
{
	return (md_vdp_get_reg(VDP_MODESET2) & VDP_MODESET2_M2) ? 240 : 224;
}

static uint16_t md_vdp_get_raster_width(void)
{
	return (md_vdp_get_reg(VDP_MODESET4) & VDP_MODESET4_RS0) ? 320 : 256;
}

static inline void md_vdp_set_sms_vl(uint8_t enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_SMSVL, enabled);
}

static inline void md_vdp_set_sms_hl(uint8_t enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_SMSHL, enabled);
}

static inline void md_vdp_set_cbus_cpu_mux(uint8_t enabled)
{
	VDP_SET(VDP_MODESET3, VDP_MODESET3_ADMUX, enabled);
}

static inline void md_vdp_set_interlace_mode(VdpInterlaceMode mode)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_LSM0, mode != VDP_INTERLACE_NONE);
	VDP_SET(VDP_MODESET4, VDP_MODESET4_LSM1, mode == VDP_INTERLACE_DOUBLE);
}

static inline void md_vdp_set_vs_clk_output(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_VSCLK, enabled);
}

static inline void md_vdp_set_hs_input(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_HSCIN, enabled);
}

static inline void md_vdp_set_spa_output(uint8_t enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_SPAEN, enabled);
}

#undef VDP_SET
#undef VDP_REG_WRITE

#endif // MD_VDP_H
