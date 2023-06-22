// This header contains static implementations of the VDP functions from vdp.h.
// It is not a header that should be included in a project, and does not have
// include guards! It has been separated like this to improve the legibility of
// vdp.h.

// =============================================================================
// VDP Function Static Implementations
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

#define VDP_DEBUG_SET(regbase, mask, en) \
do \
{ \
	MD_SYS_BARRIER();\
	if(en) \
	{ \
		md_vdp_set_debug_reg(regbase, md_vdp_get_debug_reg(regbase) | (mask)); \
	} \
	else \
	{ \
		md_vdp_set_debug_reg(regbase, md_vdp_get_debug_reg(regbase) & ~(mask)); \
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
	g_md_vdp_regs[num] = val;
	VDPPORT_CTRL = 0x8000 | (num << 8) | (val);
}

static inline uint8_t md_vdp_get_reg(uint8_t num)
{
	return g_md_vdp_regs[num];
}

static inline uint16_t md_vdp_get_status(void)
{
	return VDPPORT_CTRL;
}

void md_vdp_debug_port_sel(uint32_t num);  // vdp.inc

static inline void md_vdp_set_debug_reg(uint8_t num, uint16_t val)
{
	g_md_vdp_debug_regs[num] = val;
	md_vdp_debug_port_sel(num);
	VDPPORT_DBG_DATA = val;
}

static inline uint8_t md_vdp_get_debug_reg(uint8_t num)
{
	return g_md_vdp_debug_regs[num];
}

// Interrupt config
static inline void md_vdp_set_hint_en(bool enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_IE1, enabled);
}

static inline void md_vdp_set_vint_en(bool enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_IE0, enabled);
}

static inline void md_vdp_set_thint_en(bool enabled)
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

static inline void md_vdp_set_window_top(uint8_t cell)
{
	md_vdp_set_reg(VDP_WINVERT, cell & 0x1F);
}

static inline void md_vdp_set_window_bottom(uint8_t cell)
{
	md_vdp_set_reg(VDP_WINVERT, 0x80 | (cell & 0x1F));
}

static inline void md_vdp_set_window_right(uint8_t cell)
{
	md_vdp_set_reg(VDP_WINHORI, 0x80 | (cell & 0x1F));
}

static inline void md_vdp_set_window_left(uint8_t cell)
{
	md_vdp_set_reg(VDP_WINHORI, cell & 0x1F);
}

// Blanking
static inline void md_vdp_set_display_en(bool enabled)
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

static inline void md_vdp_set_shadow_highlight(bool enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_SHI, enabled);
}

static inline void md_vdp_set_left_column_blank(bool enabled)
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

static inline void md_vdp_set_dma_en(bool enabled)
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

static inline void md_vd_set_hv_count_latch(bool latch)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_M3, latch);
}

static inline VdpHmode md_vdp_get_hmode(void)
{
	return (md_vdp_get_reg(VDP_MODESET4) & VDP_MODESET4_RS1)
	       ? VDP_HMODE_H40 : VDP_HMODE_H32;
}

static inline VdpVmode md_vdp_get_vmode(void)
{
	return (md_vdp_get_reg(VDP_MODESET2) & VDP_MODESET2_M2)
	       ? VDP_VMODE_V30 : VDP_VMODE_V28;
}

static uint16_t md_vdp_get_raster_height(void)
{
	return (md_vdp_get_reg(VDP_MODESET2) & VDP_MODESET2_M2) ? 240 : 224;
}

static uint16_t md_vdp_get_raster_width(void)
{
	return (md_vdp_get_reg(VDP_MODESET4) & VDP_MODESET4_RS1) ? 320 : 256;
}

static inline void md_vdp_set_sms_vl(bool enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_SMSVL, enabled);
}

static inline void md_vdp_set_sms_hl(bool enabled)
{
	VDP_SET(VDP_MODESET1, VDP_MODESET1_SMSHL, enabled);
}

static inline void md_vdp_set_cbus_cpu_mux(bool enabled)
{
	VDP_SET(VDP_MODESET3, VDP_MODESET3_ADMUX, enabled);
}

static inline void md_vdp_set_interlace_mode(VdpInterlaceMode mode)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_LSM0, mode != VDP_INTERLACE_NONE);
	VDP_SET(VDP_MODESET4, VDP_MODESET4_LSM1, mode == VDP_INTERLACE_DOUBLE);
}

static inline void md_vdp_set_128k_vram_mode(bool enabled)
{
	VDP_SET(VDP_MODESET2, VDP_MODESET2_VR128, enabled);
}

static inline void md_vdp_set_vs_clk_output(bool enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_VSCLK, enabled);
}

static inline void md_vdp_set_hs_input(bool enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_HSCIN, enabled);
}

static inline void md_vdp_set_spa_output(bool enabled)
{
	VDP_SET(VDP_MODESET4, VDP_MODESET4_SPAEN, enabled);
}

static inline void md_vdp_debug_set_solo(bool enabled)
{
	VDP_DEBUG_SET(0, VDP_DBG00_SOLO, enabled);
}

static inline void md_vdp_debug_set_layer_select(VdpDebugLayerSel layer)
{
	uint16_t reg = md_vdp_get_debug_reg(0);
	reg &= ~(VDP_DBG00_LYSEL0 | VDP_DBG00_LYSEL1);
	uint8_t layer_as_int = (uint8_t)layer;
	layer_as_int &= 0x03;
	reg |= layer_as_int << 7;
	md_vdp_set_debug_reg(0, reg);
}

static inline void md_vdp_debug_set_psg_over(bool enabled, uint8_t chan)
{
	uint16_t reg = md_vdp_get_debug_reg(0);
	reg &= ~(VDP_DBG00_PSOVR | VDP_DBG00_PSOCN0 | VDP_DBG00_PSOCN1);
	reg |= (enabled ? VDP_DBG00_PSOVR : 0);
	chan &= 0x03;
	reg |= ((uint16_t)chan) << 10;
	md_vdp_set_debug_reg(0, reg);
}

static inline void md_vdp_debug_set_sprite_state_bits(uint8_t bits)
{
	uint16_t reg = md_vdp_get_debug_reg(0);
	reg &= ~(VDP_DBG00_SPRST0 | VDP_DBG00_SPRST1 | VDP_DBG00_SPRST2);
	bits &= 0x07;
	reg |= ((uint16_t)bits) << 12;
	md_vdp_set_debug_reg(0, reg);
}

static inline void md_vdp_debug_set_z80_overclock(bool enabled)
{
	VDP_DEBUG_SET(1, VDP_DBG01_Z80CK, enabled);
}

static inline void md_vdp_debug_set_edclk_out(bool enabled)
{
	VDP_DEBUG_SET(1, VDP_DBG01_EDCKO, enabled);
}

static inline uint16_t md_vdp_debug_get_bit_pattern(void)
{
	md_vdp_debug_port_sel(2);
	return VDPPORT_DBG_DATA;
}

static inline void md_vdp_debug_reset(void)
{
	md_vdp_debug_port_sel(0xF);
	VDPPORT_DBG_DATA = 0x00;
}

#undef VDP_SET
#undef VDP_SET_DEBUG

