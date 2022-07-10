/* mdk system functions
Michael Moffitt 2018-2022 */
#include "md/mmio.h"
#include "md/sys.h"
#include "md/tmss.h"

uint16_t g_md_sys_ints_enabled;

void md_sys_init(void)
{
#ifndef MDK_TARGET_C2
	md_tmss_init();
	md_sys_z80_reset_off();
	md_sys_z80_bus_req(0);
	md_sys_z80_reset_on();
	md_sys_z80_bus_release();
#endif
}

void md_sys_z80_init(const uint8_t *src, uint16_t size)
{
#ifndef MDK_TARGET_C2
	md_sys_z80_reset_off();
	md_sys_z80_bus_req(0);
	md_sys_z80_reset_on();

	volatile uint8_t *z80_ram = (volatile uint8_t *)SYS_Z80_PRG_LOC;
	for (uint16_t i = 0; i < size; i++) *z80_ram++ = *src++;

	md_sys_z80_reset_off();
	md_sys_z80_bus_release();
#else
	(void)src;
	(void)size;
#endif
}
