#ifdef MDK_TARGET_C2

#include "md/ioc.h"
#include "md/mmio.h"
#include "md/macro.h"
#include "md/io.h"

static uint8_t s_io_reg_cache[8];  // Data for ports A - H.

uint8_t g_md_c2_in[SYSC_INPUT_TYPE_COUNT];
uint8_t g_md_c2_in_prev[SYSC_INPUT_TYPE_COUNT];
uint8_t g_md_c2_in_pos[SYSC_INPUT_TYPE_COUNT];
uint8_t g_md_c2_in_neg[SYSC_INPUT_TYPE_COUNT];

// Read register data directly from Port A - Port H (0 - 7).
// Reading from registers intended for output isn't recommended.
uint8_t md_ioc_read_reg_raw(MdIoCIoPort port)
{
	volatile uint8_t *reg_porta = (volatile uint8_t *)(SYSC_IO_LOC_PORTA);
	return reg_porta[port << 1];
}

void md_ioc_set_watchdog_ctrl(bool jp15_pin3)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(SYSC_IO_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x80;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (jp15_pin3 ? 0x80 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void md_ioc_set_tda1518bq_mute(bool mute)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(SYSC_IO_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x40;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (mute ? 0x40 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void md_ioc_set_cn2_bits(bool pin10, bool pin11)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(SYSC_IO_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x30;
	s_io_reg_cache[SYSC_IO_PORT_D] |= (pin10 ? 0x20 : 0x00) | (pin11 ? 0x10 : 0x00);
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void md_ioc_set_coin_outputs(uint8_t status)
{
	volatile uint8_t *reg_portd = (volatile uint8_t *)(SYSC_IO_LOC_PORTD);
	s_io_reg_cache[SYSC_IO_PORT_D] &= ~0x0F;
	s_io_reg_cache[SYSC_IO_PORT_D] |= status;
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
}

void md_ioc_set_cn4_bits(bool a19, bool b19)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(SYSC_IO_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0xC0;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (a19 ? 0x80 : 0x00);
	s_io_reg_cache[SYSC_IO_PORT_H] |= (b19 ? 0x40 : 0x00);
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

void md_ioc_set_pal_bank(uint16_t bank)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(SYSC_IO_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0x03;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (bank & 0x0003);
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

uint16_t md_ioc_get_pal_bank(void)
{
	return s_io_reg_cache[SYSC_IO_PORT_H] & 0x0003;
}

void md_ioc_set_udp7759_bank(uint16_t bank)
{
	volatile uint8_t *reg_porth = (volatile uint8_t *)(SYSC_IO_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_H] &= ~0x3C;
	s_io_reg_cache[SYSC_IO_PORT_H] |= (bank & 0x000F) << 2;
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];
}

void md_ioc_set_upd7759_reset(bool asserted)
{
	static const uint8_t base_value = 0xF0;  // From Puyo 2 - Meaning unknown!
	volatile uint8_t *reg_ctrl2 = (volatile uint8_t *)(SYSC_IO_LOC_CTRL2);
	// As far as I can tell, CNT1 (second bit) is the reset signal.
	*reg_ctrl2 = base_value | (asserted ? 0x00 : 0x02);
}

bool md_ioc_get_upd7759_busy(void)
{
	return (g_md_c2_in[SYSC_INPUT_MISC] & SYSC_MISC_UPD7759_BUSY) ? true : false;
}


// Populates the controller data variables used for Megadrive development with
// C/C2-derived data. This exists to allow for easier patching of a game that
// targets Mega Drive to work on C/C2.
static void md_ioc_generate_compatible_input(void)
{
	typedef struct PadMapping
	{
		uint8_t c2_button;
		uint16_t md_button;
	} PadMapping;

	static const PadMapping c2_to_md_mappings[] =
	{
		{SYSC_PL_LEFT,  BTN_LEFT},
		{SYSC_PL_RIGHT, BTN_RIGHT},
		{SYSC_PL_UP,    BTN_UP},
		{SYSC_PL_DOWN,  BTN_DOWN},
		{SYSC_PL_D,     BTN_Z},
		{SYSC_PL_C,     BTN_C},
		{SYSC_PL_B,     BTN_B},
		{SYSC_PL_A,     BTN_A},
	};
	
	for (uint16_t i = 0; i < ARRAYSIZE(g_md_pad); i++)
	{
		g_md_pad_prev[i] = g_md_pad[i];
		g_md_pad[i] = 0;
		for (uint16_t j = 0; j < ARRAYSIZE(c2_to_md_mappings); j++)
		{
			if (g_md_c2_in[SYSC_INPUT_P1] &
			    c2_to_md_mappings[j].c2_button)
			{
				g_md_pad[i] |= c2_to_md_mappings[j].md_button;
			}
		}
		if (g_md_c2_in[SYSC_INPUT_SYSTEM] &
		    (i == 0 ? SYSC_SYSTEM_START1 : SYSC_SYSTEM_START2))
		{
			g_md_pad[i] |= BTN_START;
		}
	}
	
	md_io_generate_edges();
}

// Internal Use ---------------------------------------------------------------

void md_ioc_init(void)
{
	volatile uint8_t *reg_ctrl3 = (volatile uint8_t *)(SYSC_IO_LOC_CTRL3);
	*reg_ctrl3 = 0x88;  // Configure ports D and H as outputs.
	for (uint16_t i = 0; i < ARRAYSIZE(s_io_reg_cache); i++)
	{
		s_io_reg_cache[i] = 0x00;
	}
	volatile uint8_t *reg_portd = (volatile uint8_t *)(SYSC_IO_LOC_PORTD);
	volatile uint8_t *reg_porth = (volatile uint8_t *)(SYSC_IO_LOC_PORTH);
	s_io_reg_cache[SYSC_IO_PORT_D] = 0xB0;
	s_io_reg_cache[SYSC_IO_PORT_H] = 0x30;
	*reg_portd = s_io_reg_cache[SYSC_IO_PORT_D];
	*reg_porth = s_io_reg_cache[SYSC_IO_PORT_H];

	md_ioc_set_watchdog_ctrl(0);
	md_ioc_set_tda1518bq_mute(0);
	md_ioc_set_coin_outputs(0);
}

static void generate_edges(const uint8_t *fresh, const uint8_t *prev,
                           uint8_t *pos, uint8_t *neg)
{
	*pos = *fresh & ~(*prev);
	*neg = *prev & ~(*pos);
}

void md_ioc_poll(void)
{
	volatile uint8_t *reg_porta = (volatile uint8_t *)(SYSC_IO_LOC_PORTA);
	volatile uint8_t *reg_portb = (volatile uint8_t *)(SYSC_IO_LOC_PORTB);
	volatile uint8_t *reg_portc = (volatile uint8_t *)(SYSC_IO_LOC_PORTC);
	volatile uint8_t *reg_porte = (volatile uint8_t *)(SYSC_IO_LOC_PORTE);
	volatile uint8_t *reg_portf = (volatile uint8_t *)(SYSC_IO_LOC_PORTF);
	volatile uint8_t *reg_portg = (volatile uint8_t *)(SYSC_IO_LOC_PORTG);
	s_io_reg_cache[SYSC_IO_PORT_A] = ~(*reg_porta);
	s_io_reg_cache[SYSC_IO_PORT_B] = ~(*reg_portb);
	s_io_reg_cache[SYSC_IO_PORT_C] = ~(*reg_portc);
	s_io_reg_cache[SYSC_IO_PORT_E] = ~(*reg_porte);
	s_io_reg_cache[SYSC_IO_PORT_F] = ~(*reg_portf);
	s_io_reg_cache[SYSC_IO_PORT_G] = ~(*reg_portg);

	for (uint16_t i = 0; i < ARRAYSIZE(g_md_c2_in); i++)
	{
		g_md_c2_in_prev[i] = g_md_c2_in[i];
	}

	g_md_c2_in[SYSC_INPUT_P1] = s_io_reg_cache[SYSC_IO_PORT_A];
	g_md_c2_in[SYSC_INPUT_P2] = s_io_reg_cache[SYSC_IO_PORT_B];
	g_md_c2_in[SYSC_INPUT_MISC] = s_io_reg_cache[SYSC_IO_PORT_C];
	g_md_c2_in[SYSC_INPUT_SYSTEM] = s_io_reg_cache[SYSC_IO_PORT_E];
	g_md_c2_in[SYSC_INPUT_DIP1] = s_io_reg_cache[SYSC_IO_PORT_F];
	g_md_c2_in[SYSC_INPUT_DIP2] = s_io_reg_cache[SYSC_IO_PORT_G];

	for (uint16_t i = 0; i < ARRAYSIZE(g_md_c2_in); i++)
	{
		generate_edges(&g_md_c2_in[i], &g_md_c2_in_prev[i],
		               &g_md_c2_in_pos[i], &g_md_c2_in_neg[i]);
	}
	md_ioc_generate_compatible_input();
}

#endif  // MD_TARGET_C2
