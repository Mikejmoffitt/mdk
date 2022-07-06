/* mdk system functions
Michael Moffitt 2018-2022 */
#ifndef SYS_H
#define SYS_H

// Most things in this file rely on the arbiter / controller in the Mega Drive.
// System C2 does not have this IC, so most functions either work differently or
// are dummy / no-op.

#include <stdint.h>
#include "md/mmio.h"

#define MD_SYS_BARRIER() __asm__ __volatile__("": : :"memory")

extern uint16_t g_md_sys_ints_enabled;

void md_sys_init(void);

// Get system information

static inline uint8_t md_sys_is_overseas(void);
static inline uint8_t md_sys_is_pal(void);
static inline uint8_t md_sys_is_disk_present(void);
static inline uint8_t md_sys_get_hw_rev(void);

// Enable and disable interrupts.
// On System C2, the previous enablement state is stored upon a call to sys_ei()
static inline void md_sys_ei(void);
static inline void md_sys_di(void);

// Z80 control functions
void md_sys_z80_upload_program(uint8_t *src, uint16_t size);
static inline uint16_t md_sys_z80_get_bus_status(void);
static inline uint16_t md_sys_z80_get_reset_status(void);
static inline void md_sys_z80_bus_req(void);
static inline void md_sys_z80_bus_release(void);
static inline void md_sys_z80_reset_on(void);
static inline void md_sys_z80_reset_off(void);

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

// =============================================================================
// Megadrive Implementations
// =============================================================================

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

static inline uint16_t md_sys_z80_get_bus_status(void)
{
	return SYS_Z80_PORT_BUS;
}

static inline uint16_t md_sys_z80_get_reset_status(void)
{
	return SYS_Z80_PORT_RESET;
}

static inline void md_sys_z80_bus_req(void)
{
	SYS_Z80_PORT_BUS = 0x0100;
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

// =============================================================================
// System C/C2 Implementations
// =============================================================================

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

static inline void md_sys_z80_bus_req(void)
{
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

#endif // SYS_H
