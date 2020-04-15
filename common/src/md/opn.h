/* md-toolchain YM2612 support functions
Michael Moffitt 2018 */
#ifndef OPN_H
#define OPN_H

#define OPN_PORT_ADDR0 *(volatile uint8_t *)0xA04000
#define OPN_PORT_DATA0 *(volatile uint8_t *)0xA04001
#define OPN_PORT_ADDR1 *(volatile uint8_t *)0xA04002
#define OPN_PORT_DATA1 *(volatile uint8_t *)0xA04003

#define OPN_WAIT while (OPN_PORT_ADDR0 & 0x80)

static inline void opn_write(uint8_t part, uint8_t addr, uint8_t data);

static inline void opn_write(uint8_t part, uint8_t addr, uint8_t data)
{
	if (part)
	{
		OPN_WAIT
		OPN_PORT_ADDR1 = addr;
		OPN_WAIT
		OPN_PORT_DATA1 = data;
	}
	else
	{
		OPN_WAIT
		OPN_PORT_ADDR0 = addr;
		OPN_WAIT
		OPN_PORT_DATA0 = data;
	}
}

#endif // OPN_H
