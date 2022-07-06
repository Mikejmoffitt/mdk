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
#endif
}

void md_sys_z80_init(uint8_t *src, uint16_t size)
{
#ifndef MDK_TARGET_C2
	md_sys_z80_bus_req();
	while (!md_sys_z80_get_bus_status())
	{
	}
	md_sys_z80_reset_off();

	for (uint16_t i = 0; i < size; i++)
	{
		*(volatile uint8_t *)(SYS_Z80_PRG_LOC + i) = src[i];
	}

	md_sys_z80_reset_on();
	md_sys_z80_bus_release();
	md_sys_z80_reset_off();
#else
	(void)src;
	(void)size;
#endif
}
