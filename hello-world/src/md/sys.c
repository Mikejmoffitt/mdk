/* md-toolchain system functions
Michael Moffitt 2018 */
#include "md/sys.h"

uint16_t sys_ints_enabled;

void sys_z80_init(uint8_t *src, uint16_t size)
{
	sys_z80_bus_req();
	while (!sys_z80_get_bus_status())
	{
	}
	sys_z80_reset_off();

	for (uint16_t i = 0; i < size; i++)
	{
		*(volatile uint8_t *)(SYS_Z80_PRG_LOC + i) = src[i];
	}

	sys_z80_reset_on();
	sys_z80_bus_release();
	sys_z80_reset_off();
}
