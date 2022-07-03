/* md-toolchain System C/C2 I/O support
Michael Moffitt 2018-2022 */
#ifndef MD_IO_SYSTEMC_H
#define MD_IO_SYSTEMC_H

#include <stdint.h>

typedef enum SysCPlayerInput
{
	SYSC_PL_UP    = 0x80,
	SYSC_PL_DOWN  = 0x40,
	SYSC_PL_LEFT  = 0x20,
	SYSC_PL_RIGHT = 0x10,
	SYSC_PL_D     = 0x08,
	SYSC_PL_C     = 0x04,
	SYSC_PL_B     = 0x02,
	SYSC_PL_A     = 0x01,
} SysCPlayerInput;

typedef enum SysCMiscInput
{
	SYSC_MISC_MB3773P_RESET = 0x80,
	SYSC_MISC_UPD7759_BUSY  = 0x40,
	SYSC_MISC_CN2_PIN8      = 0x20,
	SYSC_MISC_CN2_PIN7      = 0x10,
	SYSC_MISC_CN2_PIN5      = 0x08,
	SYSC_MISC_CN2_PIN4      = 0x04,
	SYSC_MISC_CN2_PIN3      = 0x02,
	SYSC_MISC_CN2_PIN2      = 0x01,
} SysCMiscInput;

typedef enum SysCSystemInput
{
	SYSC_SYSTEM_SELECT  = 0x40,
	SYSC_SYSTEM_START2  = 0x20,
	SYSC_SYSTEM_START1  = 0x10,
	SYSC_SYSTEM_SERVICE = 0x08,
	SYSC_SYSTEM_TEST    = 0x04,
	SYSC_SYSTEM_COIN1   = 0x02,
	SYSC_SYSTEM_COIN2   = 0x01,
} SysCSystemInput;

typedef enum SysCDipInput
{
	SYSC_DIPSW_7 = 0x80,
	SYSC_DIPSW_6 = 0x40,
	SYSC_DIPSW_5 = 0x20,
	SYSC_DIPSW_4 = 0x10,
	SYSC_DIPSW_3 = 0x08,
	SYSC_DIPSW_2 = 0x04,
	SYSC_DIPSW_1 = 0x02,
	SYSC_DIPSW_0 = 0x01,
} SysCDipInput;

typedef enum SysCIoPort
{
	SYSC_IO_PORT_A,
	SYSC_IO_PORT_B,
	SYSC_IO_PORT_C,
	SYSC_IO_PORT_D,
	SYSC_IO_PORT_E,
	SYSC_IO_PORT_F,
	SYSC_IO_PORT_G,
	SYSC_IO_PORT_H,
} SysCIoPort;

// Reading inputs. Data is copied at start of vblank with io_systemc_poll().
// All switches are active high, with the bitfields defined above.
SysCPlayerInput io_systemc_get_player_input(int16_t player);
SysCMiscInput io_systemc_get_misc_input(void);
SysCSystemInput io_systemc_get_system_input(void);
SysCDipInput io_systemc_get_dip_input(int16_t sw);

// Read register data directly from Port A - Port H.
// Reading from registers intended for output isn't recommended.
uint8_t io_systemc_read_reg_raw(SysCIoPort port);

// Setting outputs. Last register value is cached since readback can't be done.
// Other code may use these (e.g. pal_systemc, adpcm_systemc)

void io_systemc_set_watchdog_ctrl(int16_t jp15_pin3);
void io_systemc_set_tda1518bq_mute(int16_t mute);
void io_systemc_set_cn2_bits(int16_t pin10, int16_t pin11);
void io_systemc_set_coin_outputs(int16_t lockout1, int16_t lockout2,
                                 int16_t meter1, int16_t meter2);

// Set two bits corresponding to CN4 outputs A19 and B19.
void io_systemc_set_cn4_bits(int16_t a19, int16_t b19);

// Set upper address pins for uPD7759 sample ROM A17-A18.
// Upper bits theorized to exist via Charles McDonald's doc, unconfirmed.
// Banks 0-3 valid; possibly up to 0xF.
void io_systemc_set_udp7759_bank(uint16_t bank);

// Set palette bank 0-3 through bits A9 and A10 of CRAM.
void io_systemc_set_pal_bank(uint16_t bank);

// Internal Use ---------------------------------------------------------------

// Called at startup to clear IO register caches.
void io_systemc_init(void);

// Poll controller inputs.
void io_systemc_poll(void);

#endif // MD_IO_SYSTEMC_H
