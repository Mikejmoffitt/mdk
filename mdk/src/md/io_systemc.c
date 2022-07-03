#include "md/io_systemc.h"
#include "md/mmio.h"

static uint8_t s_io_reg_cache[8];

// Read register data directly from Port A - Port H (0 - 7).
// Reading from registers intended for output isn't recommended.
uint8_t io_systemc_read_reg_raw(SysCIoPort port)
{
	volatile uint8_t *reg_porta = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTA);
	return reg_porta[port << 1];
}

// Reading inputs. Data is copied at start of vblank with io_systemc_poll().
SysCPlayerInput io_systemc_get_player_input(int16_t player)
{
	player &= 0x0001;
	return s_io_reg_cache[SYSC_IO_PORT_A + player];
}

SysCMiscInput io_systemc_get_misc_input(void)
{
	return s_io_reg_cache[SYSC_IO_PORT_C];
}

SysCSystemInput io_systemc_get_system_input(void)
{
	return s_io_reg_cache[SYSC_IO_PORT_E];
}

SysCDipInput io_systemc_get_dip_input(int16_t sw)
{
	sw &= 0x0001;
	return s_io_reg_cache[SYSC_IO_PORT_F + sw];
}

void io_systemc_set_watchdog_ctrl(int16_t jp15_pin3)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x80;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (jp15_pin3 ? 0x80 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void io_systemc_set_tda1518bq_mute(int16_t mute)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x40;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (mute ? 0x40 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void io_systemc_set_cn2_bits(int16_t pin10, int16_t pin11)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x30;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (pin10 ? 0x20 : 0x00) | (pin11 ? 0x10 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void io_systemc_set_coin_outputs(int16_t lockout1, int16_t lockout2,
                                 int16_t meter1, int16_t meter2)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x0F;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (lockout2 ? 0x08 : 0x00);
	s_io_reg_cache[SYSC_IO_PORT_D] |= (lockout1 ? 0x04 : 0x00);
	s_io_reg_cache[SYSC_IO_PORT_D] |= (meter2 ? 0x02 : 0x00);
	s_io_reg_cache[SYSC_IO_PORT_D] |= (meter1 ? 0x01 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

// Set two bits corresponding to CN4 outputs A19 and B19.
void io_systemc_set_cn4_bits(int16_t a19, int16_t b19)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0xC0;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (a19 ? 0x80 : 0x00);
	s_io_reg_cache[SYSC_IO_PORT_H] |= (b19 ? 0x40 : 0x00);
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

// Set upper address pins for uPD7759 sample ROM A17-A18.
// Upper bits theorized to exist via Charles McDonald's doc, unconfirmed.
// Banks 0-3 valid; possibly up to 0xF.
void io_systemc_set_udp7759_bank(uint16_t bank)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0x3C;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (bank & 0x000F) << 2;
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

// Set palette bank 0-3 through bits A9 and A10 of CRAM.
void io_systemc_set_pal_bank(uint16_t bank)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0x03;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (bank & 0x0003);
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

// Internal Use ---------------------------------------------------------------

void io_systemc_init(void)
{
	for (int16_t i = 0; i < 8; i++)
	{
		s_io_reg_cache[i] = 0x00;
	}
	volatile uint8_t *reg_portd = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTD);
	volatile uint8_t *reg_porth = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTH);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

// Poll controller inputs.
void io_systemc_poll(void)
{	// Read inputs.
	volatile uint8_t *reg_porta = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTA);
	volatile uint8_t *reg_portb = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTB);
	volatile uint8_t *reg_portc = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTC);
	volatile uint8_t *reg_porte = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTE);
	volatile uint8_t *reg_portf = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTF);
	volatile uint8_t *reg_portg = (volatile uint8_t *)(IO_SYSTEMC_LOC_PORTG);
	s_io_reg_cache[SYSC_IO_PORT_A] = ~(*reg_porta);
	s_io_reg_cache[SYSC_IO_PORT_B] = ~(*reg_portb);
	s_io_reg_cache[SYSC_IO_PORT_C] = ~(*reg_portc);
	s_io_reg_cache[SYSC_IO_PORT_E] = ~(*reg_porte);
	s_io_reg_cache[SYSC_IO_PORT_F] = ~(*reg_portf);
	s_io_reg_cache[SYSC_IO_PORT_G] = ~(*reg_portg);
}
