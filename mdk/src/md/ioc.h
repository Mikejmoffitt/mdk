// mdk System C/C2 I/O support
// Michael Moffitt 2018-2024
#pragma once

#ifdef MDK_TARGET_C2

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>
#include <stdbool.h>

// Input types for C/C2.

#define SYSC_PL_LEFT  0x80
#define SYSC_PL_RIGHT 0x40
#define SYSC_PL_UP    0x20
#define SYSC_PL_DOWN  0x10
#define SYSC_PL_D     0x08
#define SYSC_PL_C     0x04
#define SYSC_PL_B     0x02
#define SYSC_PL_A     0x01

#define SYSC_MISC_MB3773P_RESET 0x80
#define SYSC_MISC_UPD7759_BUSY  0x40
#define SYSC_MISC_CN2_PIN8      0x20
#define SYSC_MISC_CN2_PIN7      0x10
#define SYSC_MISC_CN2_PIN5      0x08
#define SYSC_MISC_CN2_PIN4      0x04
#define SYSC_MISC_CN2_PIN3      0x02
#define SYSC_MISC_CN2_PIN2      0x01

#define SYSC_SYSTEM_SELECT  0x40
#define SYSC_SYSTEM_START2  0x20
#define SYSC_SYSTEM_START1  0x10
#define SYSC_SYSTEM_SERVICE 0x08
#define SYSC_SYSTEM_TEST    0x04
#define SYSC_SYSTEM_COIN1   0x02
#define SYSC_SYSTEM_COIN2   0x01

#define SYSC_DIPSW_7 0x80
#define SYSC_DIPSW_6 0x40
#define SYSC_DIPSW_5 0x20
#define SYSC_DIPSW_4 0x10
#define SYSC_DIPSW_3 0x08
#define SYSC_DIPSW_2 0x04
#define SYSC_DIPSW_1 0x02
#define SYSC_DIPSW_0 0x01

#define SYSC_COIN_METER1 (1 << 0)
#define SYSC_COIN_METER2 (1 << 1)
#define SYSC_COIN_LOCKOUT1 (1 << 2)
#define SYSC_COIN_LOCKOUT2 (1 << 3)

typedef enum MdIoCIoPort
{
	SYSC_IO_PORT_A = 0,
	SYSC_IO_PORT_B = 1,
	SYSC_IO_PORT_C = 2,
	SYSC_IO_PORT_D = 3,
	SYSC_IO_PORT_E = 4,
	SYSC_IO_PORT_F = 5,
	SYSC_IO_PORT_G = 6,
	SYSC_IO_PORT_H = 7
} MdIoCIoPort;

typedef enum MdIoCInput
{
	SYSC_INPUT_P1,
	SYSC_INPUT_P2,
	SYSC_INPUT_MISC,
	SYSC_INPUT_SYSTEM,
	SYSC_INPUT_DIP1,
	SYSC_INPUT_DIP2,
	SYSC_INPUT_TYPE_COUNT
} MdIoCInput;

// Input data for the various input ports. Index with MdIoCInput.
// Data is freshly updated at the start of vblank with md_ioc_poll().
// Test for switches with the bitfields defined above.
extern uint8_t g_md_c2_in[SYSC_INPUT_TYPE_COUNT];
extern uint8_t g_md_c2_in_prev[SYSC_INPUT_TYPE_COUNT];
extern uint8_t g_md_c2_in_pos[SYSC_INPUT_TYPE_COUNT];
extern uint8_t g_md_c2_in_neg[SYSC_INPUT_TYPE_COUNT];

// Read register data directly from Port A - Port H.
// Reading from registers intended for output isn't recommended.
uint8_t md_ioc_read_reg_raw(MdIoCIoPort port);

// Setting outputs. Last register value is cached since readback can't be done.
// Other code may use these (e.g. pal_systemc, adpcm_systemc)

// Controls pin 3 of JP15.
void md_ioc_set_watchdog_ctrl(bool jp15_pin3);

// Mutes the audio amplifier.
void md_ioc_set_tda1518bq_mute(bool mute);

// Controls pin 10 and pin 11 of CN2.
void md_ioc_set_cn2_bits(bool pin10, bool pin11);

// Sets the coin meter and coil driver outputs. Use SYSC_COIN_* definitions.
void md_ioc_set_coin_outputs(uint8_t state);

// Set two bits corresponding to CN4 outputs A19 and B19.
void md_ioc_set_cn4_bits(bool a19, bool b19);

// Set upper address pins for uPD7759 sample ROM A17-A18.
// Upper bits theorized to exist via Charles McDonald's doc, unconfirmed.
// Banks 0-3 valid; possibly up to 0xF.
void md_ioc_set_udp7759_bank(uint16_t bank);

// Assert (drive to 0) or Deassert (raise to 1) the reset pin for the uPD7759.
void md_ioc_set_upd7759_reset(bool asserted);

// Returns true if the uPD7759 reports it is busy.
bool md_ioc_get_upd7759_busy(void);

// Set global palette bank 0-3 through bits A9 and A10 of CRAM.
void md_ioc_set_pal_bank(uint16_t bank);

// Returns the global palette bank.
uint16_t md_ioc_get_pal_bank(void);

// Internal Use ---------------------------------------------------------------

// Called at startup to clear IO register caches.
void md_ioc_init(void);

// Poll controller inputs.
void md_ioc_poll(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MDK_TARGET_C2
