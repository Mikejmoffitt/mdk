// mdk YM2612 support functions
// Michael Moffitt 2018-2022
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#define OPN_PORT_ADDR0 *(volatile uint8_t *)(OPN_BASE+0)
#define OPN_PORT_DATA0 *(volatile uint8_t *)(OPN_BASE+1)
#define OPN_PORT_ADDR1 *(volatile uint8_t *)(OPN_BASE+2)
#define OPN_PORT_DATA1 *(volatile uint8_t *)(OPN_BASE+3)

#define OPN_WAIT while (OPN_PORT_ADDR0 & 0x80) { continue; }

static inline void md_opn_write(uint8_t part, uint8_t addr, uint8_t data)
{
	if (part)
	{
		OPN_PORT_ADDR1 = addr;
		OPN_WAIT
		OPN_PORT_DATA1 = data;
	}
	else
	{
		OPN_PORT_ADDR0 = addr;
		OPN_WAIT
		OPN_PORT_DATA0 = data;
	}
}

#ifdef __cplusplus
}
#endif  // __cplusplus
