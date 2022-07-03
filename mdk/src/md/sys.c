/* md-toolchain system functions
Michael Moffitt 2018 */
#include "md/mmio.h"
#include "md/sys.h"

uint16_t g_sys_ints_enabled;
MdSystemType g_sys_type = SYSTEM_TYPE_UNKNOWN;

void sys_init(void)
{
	// Figure out if we're on C/C2 or Megadrive. I suppose you could fool this
	// if you had a 32X attached and a very specific value placed in the FB.

	/*
	const volatile uint8_t *reg_protection = (volatile uint8_t *)0x840011;
	const uint8_t test_signature[4] = {'S', 'E', 'G', 'A'};
	for (int16_t i = 0; i < 4; i++)
	{
		const uint8_t reg_read = reg_protection[i * 2];
		SYS_BARRIER();
		// TODO: I guess it's possible we're not running on ANY of these...
		if (reg_read != test_signature[i])
		{
			g_sys_type = SYSTEM_TYPE_MD;
			return;
		}
	}*/
	g_sys_type = SYSTEM_TYPE_C;
}

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
