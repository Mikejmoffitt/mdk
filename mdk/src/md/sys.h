// mdk system functions
// Michael Moffitt 2018-2022
//
// Most things in this file rely on the arbiter / controller in the Mega Drive.
// System C2 does not have this IC, so most functions either work differently or
// are dummy / no-op.
#ifndef MD_SYS_H
#define MD_SYS_H

#include <stdint.h>
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
static inline void md_sys_ei(void);
// Disable interrupts.
static inline void md_sys_di(void);

// Returns true if interrupts are enabled. Useful for storing and restoring the
// interrupt enablement status before and after changing them.
static uint16_t md_sys_get_ints_enabled(void);

// -----------------------------------------------------------------------------
// Z80 control functions
// -----------------------------------------------------------------------------

// Uploads a binary blob to the Z80 program memory. Holds the Z80 bus, but does
// not reset the z80.
void md_sys_z80_init(const uint8_t *src, uint16_t size);

// Requests access to the Z80 bus, halting it. Can block until granted.
static inline void md_sys_z80_bus_req(uint8_t wait);
static inline void md_sys_z80_bus_release(void);
static inline void md_sys_z80_reset_on(void);
static inline void md_sys_z80_reset_off(void);

// -----------------------------------------------------------------------------
// System information
// -----------------------------------------------------------------------------
static inline uint8_t md_sys_is_overseas(void);
static inline uint8_t md_sys_is_pal(void);
static inline uint8_t md_sys_is_disk_present(void);
static inline uint8_t md_sys_get_hw_rev(void);

// =============================================================================
// Implementations
// =============================================================================
extern uint16_t g_md_sys_ints_enabled;

static inline void md_sys_ei(void)
{
	__asm__ volatile("\tandi.w	#0xF8FF, %sr\n");
	g_md_sys_ints_enabled = 1;
}

static inline void md_sys_di(void)
{
	__asm__ volatile("\tori.w	#0x0700, %sr\n");
	g_md_sys_ints_enabled = 0;
}

static uint16_t md_sys_get_ints_enabled(void)
{
	return g_md_sys_ints_enabled;
}

// -----------------------------------------------------------------------------
// Megadrive Implementations
// -----------------------------------------------------------------------------

#ifndef MDK_TARGET_C2

static inline uint8_t md_sys_is_overseas(void)
{
	return (SYS_PORT_VERSION & 0x80) ? 1 : 0;
}

static inline uint8_t md_sys_is_pal(void)
{
	return (SYS_PORT_VERSION & 0x40) ? 1 : 0;
}

static inline uint8_t md_sys_is_disk_present(void)
{
	return (SYS_PORT_VERSION & 0x20) ? 1 : 0;
}

static inline uint8_t md_sys_get_hw_rev(void)
{
	return SYS_PORT_VERSION & 0x0F;
}

static inline void md_sys_z80_bus_req(uint8_t wait)
{
	SYS_Z80_PORT_BUS = 0x0100;
	if (!wait) return;
	MD_SYS_BARRIER();
	while ((SYS_Z80_PORT_BUS & 0x0100)) { __asm__("nop"); }
}

static inline void md_sys_z80_bus_release(void)
{
	SYS_Z80_PORT_BUS = 0x0000;
}

static inline void md_sys_z80_reset_on(void)
{
	SYS_Z80_PORT_RESET = 0x0100;
}

static inline void md_sys_z80_reset_off(void)
{
	SYS_Z80_PORT_RESET = 0x0000;
}

// -----------------------------------------------------------------------------
// System C/C2 Implementations
// -----------------------------------------------------------------------------

#else

static inline uint8_t md_sys_is_overseas(void)
{
	return 0;
}

static inline uint8_t md_sys_is_pal(void)
{
	return 0;
}

static inline uint8_t md_sys_is_disk_present(void)
{
	return 0;
}

static inline uint8_t md_sys_get_hw_rev(void)
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

static inline void md_sys_z80_bus_req(uint8_t wait)
{
	(void)wait;
}

static inline void md_sys_z80_bus_release(void)
{
}

static inline void md_sys_z80_reset_on(void)
{
}

static inline void md_sys_z80_reset_off(void)
{
}

#endif  // MDK_TARGET_C2

#endif // MD_SYS_H
