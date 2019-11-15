/* md-toolchain I/O peripheral support
Michael Moffitt 2018 */
#include "md/io.h"
#include "md/sys.h"

static uint16_t pad_cache[3];

void io_poll(void)
{
	volatile uint8_t *port_data1 = (volatile uint8_t *)(IO_LOC_DATA1);
	volatile uint8_t *port_data2 = (volatile uint8_t *)(IO_LOC_DATA2);
	volatile uint8_t *port_data3 = (volatile uint8_t *)(IO_LOC_DATA3);
	sys_z80_bus_req();
	*port_data1 = 0x40;
	*port_data2 = 0x40;
	*port_data3 = 0x40;
	__asm__ volatile ("\tnop\n");
	__asm__ volatile ("\tnop\n");
	__asm__ volatile ("\tnop\n");
	pad_cache[0] = *port_data1 & 0x3F;
	pad_cache[1] = *port_data2 & 0x3F;
	pad_cache[2] = *port_data3 & 0x3F;
	*port_data1 = 0x00;
	*port_data2 = 0x00;
	*port_data3 = 0x00;
	__asm__ volatile ("\tnop\n");
	__asm__ volatile ("\tnop\n");
	__asm__ volatile ("\tnop\n");
	pad_cache[0] |= ((*port_data1 & (0x30)) << 2);
	pad_cache[1] |= ((*port_data2 & (0x30)) << 2);
	pad_cache[2] |= ((*port_data3 & (0x30)) << 2);
	sys_z80_bus_release();
}

uint16_t io_pad_read(uint8_t port)
{
	if (port > 2)
	{
		return 0x0000;
	}
	return ~pad_cache[port];
}

void io_thint_en(uint8_t port, uint8_t enabled)
{
	if (port > 2)
	{
		return;
	}
	volatile uint8_t *port_ctrl = (volatile uint8_t *)(IO_LOC_CTRL1+(port*2));
	*port_ctrl = enabled ? 0xC0 : 0x40;
}

void io_gamepad_en(uint8_t port)
{
	if (port > 2)
	{
		return;
	}
	volatile uint8_t *port_ctrl = (volatile uint8_t *)(IO_LOC_CTRL1+(port*2));
	*port_ctrl = 0x40; // Enable TH pin as output to multiplex buttons.
}

