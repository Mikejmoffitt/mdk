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

	static const uint8_t s_z80_stub_program[] =
	{
		0xF3,       // di
		0x18, 0xFE  // jr $
	};
	md_sys_z80_init(s_z80_stub_program, sizeof(s_z80_stub_program));
#endif
}

void md_sys_z80_init(const uint8_t *src, uint16_t size)
{
#ifndef MDK_TARGET_C2
	md_sys_z80_reset_assert();
	md_sys_z80_bus_req(/*wait=*/0);
	md_sys_z80_reset_deassert();

	volatile uint8_t *z80_ram = (volatile uint8_t *)SYS_Z80_PRG_LOC;
	for (uint16_t i = 0; i < size; i++) *z80_ram++ = *src++;

	md_sys_z80_reset_assert();
	// Long reset for the YM2612.
	for (uint16_t i = 0; i < 32; i++) __asm__("nop");

	md_sys_z80_reset_deassert();
	md_sys_z80_bus_release();
#else
	(void)src;
	(void)size;
#endif
}
