/* md-toolchain system functions
Michael Moffitt 2018 */
#ifndef SYS_H
#define SYS_H

#include <stdint.h>
#include "md/mmio.h"

#define SYS_BARRIER() __asm__ __volatile__("": : :"memory")

typedef enum MdSystemType
{
	SYSTEM_TYPE_UNKNOWN,
	SYSTEM_TYPE_MD,
	SYSTEM_TYPE_C,
} MdSystemType;

extern uint16_t g_sys_ints_enabled;
extern MdSystemType g_sys_type;

void sys_init(void);

// Get system information

static inline MdSystemType sys_get_type(void);
MdSystemType sys_get_type(void);
static inline uint8_t sys_is_overseas(void);
static inline uint8_t sys_is_pal(void);
static inline uint8_t sys_is_disk_present(void);
static inline uint8_t sys_get_hw_rev(void);

// Enable and disable interrupts
static inline void sys_ei(void);
static inline void sys_di(void);

// Z80 control functions
void sys_z80_upload_program(uint8_t *src, uint16_t size);
static inline uint16_t sys_z80_get_bus_status(void);
static inline uint16_t sys_z80_get_reset_status(void);
static inline void sys_z80_bus_req(void);
static inline void sys_z80_bus_release(void);
static inline void sys_z80_reset_on(void);
static inline void sys_z80_reset_off(void);

static inline MdSystemType sys_get_type(void)
{
	return g_sys_type;
}

static inline uint8_t sys_is_overseas(void)
{
	return (SYS_PORT_VERSION & 0x80) ? 1 : 0;
}

static inline uint8_t sys_is_pal(void)
{
	return (SYS_PORT_VERSION & 0x40) ? 1 : 0;
}

static inline uint8_t sys_is_disk_present(void)
{
	return (SYS_PORT_VERSION & 0x20) ? 1 : 0;
}

static inline uint8_t sys_get_hw_rev(void)
{
	return SYS_PORT_VERSION & 0x0F;
}

static inline void sys_ei(void)
{
	__asm__ volatile("\tandi.w	#0xF8FF, %sr\n");
	g_sys_ints_enabled = 1;
}

static inline void sys_di(void)
{
	__asm__ volatile("\tori.w	#0x700, %sr\n");
	g_sys_ints_enabled = 0;
}

static uint16_t sys_get_ints_enabled(void)
{
	return g_sys_ints_enabled;
}

static inline uint16_t sys_z80_get_bus_status(void)
{
	return SYS_Z80_PORT_BUS;
}

static inline uint16_t sys_z80_get_reset_status(void)
{
	return SYS_Z80_PORT_RESET;
}

static inline void sys_z80_bus_req(void)
{
	SYS_Z80_PORT_BUS = 0x0100;
}

static inline void sys_z80_bus_release(void)
{
	SYS_Z80_PORT_BUS = 0x0000;
}

static inline void sys_z80_reset_on(void)
{
	SYS_Z80_PORT_RESET = 0x0100;
}

static inline void sys_z80_reset_off(void)
{
	SYS_Z80_PORT_RESET = 0x0000;
}

#endif // SYS_H
