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

// TODO: Deprecate storage of some of these registers that have no reason to
// be read (e.g. DMA parameters). Modeset, plane size will be kept so that
// specific accessor functions will work.

#ifndef MD_VDP_H
#define MD_VDP_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdbool.h>
#include <stdint.h>

#include "md/mmio.h"                  // Memory map, including VDP.
#include "md/sys.h"                   // For MD_SYS_BARRIER().
#include "md/vdp_regs.h"              // Register names and bit values.
#include "md/vdp_default_vram_map.h"  // Default layout of video RAM.
#include "md/vdp_enums.h"             // Enums used for VDP access functions.

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

// Tile / sprite attribute definition macro.
#define VDP_ATTR(_tile, _hflip, _vflip, _pal, _prio) (((_tile) & 0x7FF) | \
                 ((_hflip) ? 0x800 : 0) | ((_vflip) ? 0x1000 : 0) | \
                 (((_pal) & 0x3) << 13) | ((_prio) ? 0x8000 : 0))

// Macro to form command to set an address via the control port.
#define VDP_CTRL_ADDR(_addr) ((((uint32_t)(_addr) & 0x3FFF) << 16) | \
                               (((uint32_t)(_addr) & 0xC000) >> 14))

// The register cache.
extern uint8_t g_md_vdp_regs[0x18];
extern uint16_t g_md_vdp_debug_regs[0x10];

// =============================================================================
// Functions and accessors
// =============================================================================
void md_vdp_init(void);

void md_vdp_vram_clear(void);

// -----------------------------------------------------------------------------
// Generic Accessors
// -----------------------------------------------------------------------------

// Sets a full register value, and updates cache accordingly.
static inline void md_vdp_set_reg(uint8_t num, uint8_t val);

// Returns the last register value from the cache.
static inline uint8_t md_vdp_get_reg(uint8_t num);

// Returns the VDP status register contents.
static inline uint16_t md_vdp_get_status(void);

// Write to the debug register.
static inline void md_vdp_set_debug_reg(uint8_t num, uint16_t val);

// -----------------------------------------------------------------------------
// Interrupt config
// -----------------------------------------------------------------------------

// The VDP's interrupt sources. Handlers may be registered for these interrupts
// in `irq.h` as normal callback functions.
// Even if these sources are enabled, interrupts may be masked with sys_di().

// Enables an IRQ that fires at the start of vertical blank, once per frame.
static inline void md_vdp_set_vint_en(bool enabled);
// Enables an IRQ that fires upon transition of the controller's TH pin.
static inline void md_vdp_set_thint_en(bool enabled);
// Enables an IRQ that fires at the horizontal blank of a scanline.
static inline void md_vdp_set_hint_en(bool enabled);
// Set the number of scanlines between horizontal interrupts.
static inline void md_vdp_set_hint_line(uint8_t line);

// Upon calling this, the CPU will wait until the vertical interrupt is fired,
// at which point this routine will return. This is dependent on the vertical
// interrupt being enabled, so do not call this if those have been disabled!
void md_vdp_wait_vblank(void);

// Register a function to call while waiting for vblank. This is useful for any
// code that handles work progressively across frames, where variable
// of CPU time is acceptable.
// Pass NULL in for function to remove a callback.
void md_vdp_register_vblank_wait_callback(void (*function)(void));

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

// Enable the window plane, and have it draw from the column/line specified with
// `cell`. Call any of these with 0 to disable the horizontal or vertical window.
static inline void md_vdp_set_window_top(uint8_t cell);
static inline void md_vdp_set_window_bottom(uint8_t cell);
static inline void md_vdp_set_window_right(uint8_t width);
static inline void md_vdp_set_window_left(uint8_t width);

// -----------------------------------------------------------------------------
// Raster config
// -----------------------------------------------------------------------------

// Sets the display output on or off. If turned off, the VDP will just output
// the color pointed to by the background color index (md_vdp_set_bg_color()).
static inline void md_vdp_set_display_en(bool enabled);

// Set the "background color" output in the display's blanking region. This
// refers to an index in color RAM, so the color seen depends on the palette.
static inline void md_vdp_set_bg_color(uint8_t idx);

// Set the display resolution. VDP_VMODE_V30 is only available for PAL/50Hz.)
static inline void md_vdp_set_hmode(VdpHmode mode);
static inline void md_vdp_set_vmode(VdpVmode mode);
static inline VdpHmode md_vdp_get_hmode(void);
static inline VdpVmode md_vdp_get_vmode(void);

// Get the size of the display in pixels. Dependent on H and V modes set above.
static uint16_t md_vdp_get_raster_width(void);
static uint16_t md_vdp_get_raster_height(void);

// Enable shadow/highlight mode. In this mode, the priority bit for the scroll
// darkens the cell when cleared. Sprites will be shaded by the background cell
// if their priority bit is not set. Furthermore, palette entries $3E and $3F
// output a special "highlight" and "shadow" color respectively when used by
// sprites.
static inline void md_vdp_set_shadow_highlight(bool enabled);

// Blanks the leftmost eight columns of the screen to the background color.
static inline void md_vdp_set_left_column_blank(bool enabled);

// Enable or disable one of the interlaced modes.
static inline void md_vdp_set_interlace_mode(VdpInterlaceMode mode);

// Enables 128K VRAM mode. Not advised for stock MD / Genesis / C2 systems.
static inline void md_vdp_set_128k_vram_mode(bool enabled);

// Output a dot clock signal on the VDP's vertical sync pin. This is used as a
// pixel data latch on System 18 and System C/C2.
static inline void md_vdp_set_vs_clk_output(bool enabled);

// Enable output of the sprite/background pixel indicator pin on the VDP. This
// is used for color RAM addressing on System 18 and System C/C2.
static inline void md_vdp_set_spa_output(bool enabled);

// When set, the VDP will output the pixel color index on the color bus pins.
// When cleared, the VDP allows the CPU address to drive those pins.
// System C/C2 use this to write to Color RAM. It should be set during active
// display, and only cleared when external Color RAM is being written to.
static inline void md_vdp_set_cbus_cpu_mux(bool enabled);

// Turns the VDP's horizontal sync pin into an input. This will cause a loss
// of sync on a standard Mega Drive / Genesis.
static inline void md_vdp_set_hs_input(bool enabled);

// Set the (obsolete) vertical and horizontal lock registers, which are
// legacy registers carried over from the SMS.
static inline void md_vdp_set_sms_vl(bool enabled);
static inline void md_vdp_set_sms_hl(bool enabled);

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
static inline void md_vd_set_hv_count_latch(bool latch);

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
static inline void md_vdp_set_dma_en(bool enabled);

// Blocks until the VDP status register indicates a DMA is no longer in
// progress. Used by `dma.c`.
static inline void md_vdp_wait_dma(void);

// -----------------------------------------------------------------------------
// Debug registers
// -----------------------------------------------------------------------------

// Enables SOLO layer mode. The layer to be displayed is selected with the LYSEL
// register bits, set with md_vdp_debug_set_layer_select().
// If this feature is enabled, all blanking logic is disabled, so the borders
// of the display will contain garbage data that is not normally displayed.
// This may include fetch data for nametables, sprite data, etc.
static inline void md_vdp_debug_set_solo(bool enabled);

// Selects a layer to display for SOLO mode with md_vdp_debug_set_solo().
// If a choice other than VDP_DEBUG_LYSEL_NONE(0) is chosen while SOLO is not
// enabled, the VDP will still force the display of that layer. The chosen
// layer's pixel data will interfere with the pixel data of the VDP's normal
// output. This typically results in the pixel values being logically ANDed
// with one another, which may be used to interesting effect.
// Note that this phenomenon occurs at a hardware level, likely due to a bus
// conflict, so results may vary based on hardware, revision, temperature,
// voltage, luck, cosmic rays, fortune, and karma. It might be bad for the VDP.
static inline void md_vdp_debug_set_layer_select(VdpDebugLayerSel layer);

// If enabled, all PSG channels use the attenuation value from one channel.
static inline void md_vdp_debug_set_psg_over(bool enabled, uint8_t chan);

// Sets sprite render state bits 0-2 (range 0-7). The meaning of each bit and
// consequent impact are not yet known.
static inline void md_vdp_debug_set_sprite_state_bits(uint8_t bits);

// Doubles the Z80 and PSG clocks to about 7.67Mhz. This may not be stable on
// units fitted with old NMOS Z80s rated for 4MHz, so it is not recommended that
// your software rely on this functionality.
// As a side effect, the PSG clock will be doubled, increasing the pitch by
// an octave.
static inline void md_vdp_debug_set_z80_overclock(bool enabled);

// Configures the EDCLK pin as an output, which outputs the dot clock. This is
// not useful on the Megadrive, as EDCLK is supplied with a clock.
static inline void md_vdp_debug_set_edclk_out(bool enabled);

// Returns a walking bit pattern in the lower byte.
static inline uint16_t md_vdp_debug_get_bit_pattern(void);

// Resets the VDP.
static inline void md_vdp_debug_reset(void);

#include "md/vdp_static_impl.inc"

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // MD_VDP_H
