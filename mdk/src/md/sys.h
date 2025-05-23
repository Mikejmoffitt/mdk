// mdk system functions
// Michael Moffitt 2018-2024
//
// Most things in this file rely on the arbiter / controller in the Mega Drive.
// System C2 does not have this IC, so most functions either work differently or
// are dummy / no-op.
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>
#include <stdbool.h>
#include "md/mmio.h"

// =============================================================================
// Interface
// =============================================================================

// If on Mega Drive / Genesis, initializes TMSS if applicable.
void md_sys_init(void);

// Inserts a memory barrier.
#define MD_SYS_BARRIER() __asm__ __volatile__("": : :"memory")

// -----------------------------------------------------------------------------
// Interrupts.
// -----------------------------------------------------------------------------

// Enable interrupts.
static inline uint8_t md_sys_ei(void);
// Disable interrupts. Returns the previous interrupt enablement status, so that
// they may be conditionally re-enabled later.
static inline uint8_t md_sys_di(void);

// -----------------------------------------------------------------------------
// Z80 control functions
// -----------------------------------------------------------------------------

// Uploads a binary blob to the Z80 program memory. Holds the Z80 bus, but does
// not reset the z80.
void md_sys_z80_init(const uint8_t *src, uint16_t size);

// Requests access to the Z80 bus, halting it. Can block until granted.
static inline void md_sys_z80_bus_req(bool wait);
static inline void md_sys_z80_bus_release(void);
static inline bool md_sys_z80_bus_taken(void);
// Set the Z80 reset line high or low. Low (asserted) holds it and the YM2612
// in a reset state.
static inline void md_sys_z80_reset_deassert(void);
static inline void md_sys_z80_reset_assert(void);

// -----------------------------------------------------------------------------
// System information
// -----------------------------------------------------------------------------
static inline bool md_sys_is_overseas(void);
static inline bool md_sys_is_pal(void);
static inline bool md_sys_is_disk_present(void);
static inline uint8_t md_sys_get_hw_rev(void);

// =============================================================================
// Implementations
// =============================================================================
extern bool g_md_sys_ints_enabled;

static inline uint8_t md_sys_ei(void)
{
#ifdef MDK_TARGET_C2
	__asm__ volatile("\tandi.w	#0xFCFF, %sr\n");
#else
	__asm__ volatile("\tandi.w	#0xF8FF, %sr\n");
#endif
	const uint8_t ret = g_md_sys_ints_enabled;
	g_md_sys_ints_enabled = true;
	return ret;
}

static inline uint8_t md_sys_di(void)
{
#ifdef MDK_TARGET_C2
	__asm__ volatile("\tori.w	#0x0700, %sr\n");
#else
	__asm__ volatile("\tori.w	#0x0700, %sr\n");
#endif
	const uint8_t ret = g_md_sys_ints_enabled;
	g_md_sys_ints_enabled = false;
	return ret;
}

// -----------------------------------------------------------------------------
// Megadrive Implementations
// -----------------------------------------------------------------------------

#ifndef MDK_TARGET_C2

static inline bool md_sys_is_overseas(void)
{
	return (SYS_PORT_VERSION & 0x80) ? true : false;
}

static inline bool md_sys_is_pal(void)
{
	return (SYS_PORT_VERSION & 0x40) ? true : false;
}

static inline bool md_sys_is_disk_present(void)
{
	return (SYS_PORT_VERSION & 0x20) ? true : false;
}

static inline uint8_t md_sys_get_hw_rev(void)
{
	return SYS_PORT_VERSION & 0x0F;
}

static inline bool md_sys_z80_bus_taken(void)
{
	volatile uint8_t *z80_bus = (volatile uint8_t *)SYS_Z80_PORT_BUS_LOC;
	return !(*z80_bus & 0x01);
}

static inline void md_sys_z80_bus_req(bool wait)
{
	volatile uint8_t *z80_bus = (volatile uint8_t *)SYS_Z80_PORT_BUS_LOC;

	*z80_bus = 0x01;
	if (!wait) return;
	while (*z80_bus & 0x01) __asm__("nop");
}

static inline void md_sys_z80_bus_release(void)
{
	volatile uint8_t *z80_bus = (volatile uint8_t *)SYS_Z80_PORT_BUS_LOC;
	*z80_bus = 0x00;
}

static inline void md_sys_z80_reset_deassert(void)
{
	volatile uint8_t *z80_reset = (volatile uint8_t *)SYS_Z80_PORT_RESET_LOC;
	*z80_reset = 0x01;
}

static inline void md_sys_z80_reset_assert(void)
{
	volatile uint8_t *z80_reset = (volatile uint8_t *)SYS_Z80_PORT_RESET_LOC;
	*z80_reset = 0x00;
}

// -----------------------------------------------------------------------------
// System C/C2 Implementations
// -----------------------------------------------------------------------------

#else

static inline bool md_sys_is_overseas(void)
{
	return false;
}

static inline bool md_sys_is_pal(void)
{
	return false;
}

static inline bool md_sys_is_disk_present(void)
{
	return false;
}

static inline uint8_t md_sys_get_hw_rev(void)
{
	return 0;
}

static inline bool md_sys_z80_bus_taken(void)
{
	return 0;
}

// There's no Z80.
static inline uint16_t md_sys_z80_get_bus_status(void)
{
	return 0;
}

static inline uint16_t md_sys_z80_get_reset_status(void)
{
	return 0;
}

static inline void md_sys_z80_bus_req(bool wait)
{
	(void)wait;
}

static inline void md_sys_z80_bus_release(void)
{
}

static inline void md_sys_z80_reset_deassert(void)
{
}

static inline void md_sys_z80_reset_assert(void)
{
}

#endif  // MDK_TARGET_C2

#ifdef __cplusplus
}
#endif  // __cplusplus
