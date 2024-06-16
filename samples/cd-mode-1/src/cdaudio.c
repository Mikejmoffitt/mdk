// Sega CD Mode 1 support code for SGDK
// Michael Moffitt (https://github.com/mikejmoffitt)
// Adapted from Chilly Willy's Mode 1 CD player

#include "cdaudio.h"
#include "md/megadrive.h"
#include <string.h>

typedef struct cd_info cd_info;
struct cd_info
{
	uint16_t bios_stat;
	uint8_t first_tno;
	uint8_t last_tno;
	uint8_t drv_vers;
	uint8_t flag;
};

static cd_info info;

/* ----- Hardware access utils ----- */
volatile uint8_t *segacd_bios_addr;
extern uint32_t Sub_Start;
extern uint32_t Sub_End;
extern void Kos_Decomp(volatile uint8_t *src, volatile uint8_t *dst);
//extern int16_t set_sr(int16_t new_sr);

static volatile uint16_t *reg_cpu = (volatile uint16_t *)SEGACD_REG_CPU;
static volatile uint16_t *reg_mem = (volatile uint16_t *)SEGACD_REG_MEM;
static volatile uint16_t *reg_hint = (volatile uint16_t *)SEGACD_REG_HINT;
static volatile uint16_t *reg_stopwatch = (volatile uint16_t *)SEGACD_REG_STOPWATCH;
static volatile uint8_t *reg_comm_r = (volatile uint8_t *)(SEGACD_REG_COMM + 1);
static volatile uint8_t *reg_comm_w = (volatile uint8_t *)(SEGACD_REG_COMM);

static volatile uint16_t *track_w = (volatile uint16_t *)SEGACD_TRACK_W;
static volatile uint8_t *pmode_w = (volatile uint8_t *)SEGACD_PMODE_W;

static volatile uint16_t *bios_stat_ptr = (volatile uint16_t *)SEGACD_BIOS_STAT;
static volatile uint8_t *first_tno_ptr = (volatile uint8_t *)SEGACD_FIRST_TNO;
static volatile uint8_t *last_tno_ptr = (volatile uint8_t *)SEGACD_LAST_TNO;
static volatile uint8_t *drv_vers_ptr = (volatile uint8_t *)SEGACD_DRV_VERS;
static volatile uint8_t *flag_ptr = (volatile uint8_t *)SEGACD_FLAG;

// Write to writeonly COMM port; upper 8 bits
static inline void comm_write(uint8_t msg)
{
	*reg_comm_w = msg;
}

// Read from readonly COMM port; lower 8 bits
static inline char comm_read(void)
{
	return *reg_comm_r;
}

static inline int16_t cdaudio_get_status(void)
{
	return (info.bios_stat >> 8);
}

// Block until sub-CPU bus is requested
static inline void bus_req(void)
{
	while ((*reg_cpu & 0x0002) == 0)
	{
		*reg_cpu = (*reg_cpu & 0xFF00) | 0x02;
	}
}

// Block until sub-CPU acknowledges command
static uint8_t wait_cmd_ack(void)
{
	uint8_t ack = 0;

	while (!ack)
	{
		ack = comm_read();
	}

	return ack;
}

// Do a command, blocking until it has been accepted
static void wait_do_cmd(uint8_t cmd)
{
	while (comm_read())
	{
		__asm__("nop");
	}
	comm_write(cmd);
}

static inline void wait_cpu_running(void)
{
	while ((*reg_cpu & 0x0001) == 0)
	{
		*reg_cpu = (*reg_cpu & 0xFF00) | 0x01;
	}
}

static inline void delay(int32_t iterations)
{
	while (iterations--)
	{
		__asm__("nop");
	}
}

/* ----- SegaCD initialization support functions ----- */

// Check for the CD BIOS
// Returns zero if a Sega CD was not detected. Returns location of Sega CD BIOS
// on success.
static volatile uint8_t *check_hardware(void)
{
	volatile uint8_t *bios;

	bios = (volatile uint8_t *)SCD_BIOSLOC_1;
	if (memcmp(bios + SCD_OFFSET, "SEGA", 4))
	{
		bios = (volatile uint8_t *)SCD_BIOSLOC_2;
		if (memcmp(bios + SCD_OFFSET, "SEGA", 4))
		{
			// Wondermega or X'Eye
			if (memcmp(bios + SCD_OFFSET, "WONDER", 6))
			{
				bios = (volatile uint8_t *)SCD_BIOSLOC_3;
				// Laseractive check
				if (memcmp(bios + SCD_OFFSET, "SEGA", 4))
				{
					return 0;
				}
			}
		}
	}
	return bios;
}

// Needed to clear Laseractive internal state, apparently
static void reset_gate_array(void)
{
	volatile uint8_t *loc2;
	loc2 = (volatile uint8_t *)0xA12001;

	*reg_mem = 0xFF00;
	*loc2 = 0x03;
	*loc2 = 0x02;
	*loc2 = 0x00;
}

// Reset Sub-CPU and req bus
static void subcpu_setup(void)
{
	bus_req();
}

static int32_t decompress_bios(void)
{
	// Configure for writing
	*reg_mem = 0x0002;

	// Clear first bank of program RAM for laseractive
	memset((void *)SEGACD_PROGRAM_ADDR, 0, SEGACD_PROGRAM_LEN);

	// Decompress BIOS here
	Kos_Decomp(segacd_bios_addr, (uint8_t *)SEGACD_PROGRAM_ADDR);

	// Copy program to program RAM
	memcpy((void *)SEGACD_PROGRAM_OFF, &Sub_Start, (int)&Sub_End - (int)&Sub_Start);
	return (memcmp((void *)SEGACD_PROGRAM_OFF, &Sub_Start, (int)&Sub_End - (int)&Sub_Start)) ? 0 : 1;
}

void cdaudio_check_disc(void)
{
	wait_do_cmd('C');
	wait_cmd_ack();
	comm_write(SEGACD_CMD_ACK);
}


static char get_disc_info(void)
{
	char ack;
	wait_do_cmd('D');
	ack = wait_cmd_ack();
	comm_write(SEGACD_CMD_ACK);
	info.bios_stat = *bios_stat_ptr;
	info.first_tno = *first_tno_ptr;
	info.last_tno = *last_tno_ptr;
	info.drv_vers = *drv_vers_ptr;
	info.flag = *flag_ptr;
	return ack;
}
// Wait until the player is done reading the TOC or scanning
static void wait_for_ready(void)
{
	uint8_t ack;
	uint8_t ok = 0;
	while (!ok)
	{
		ack = get_disc_info();
		if (ack == 'D')
		{
			switch (cdaudio_get_status())
			{
				case SEGACD_STAT_SCANNING:
				case SEGACD_STAT_READINGTOC:
					ok = 0;
					break;
				default:
					if (info.bios_stat & 0x8000)
					{
						ok = 0;
						break;
					}
					ok = 1;
					break;
			}
		}
	}
}

static void start_subcpu(void)
{
	comm_write(SEGACD_CMD_ACK); // Clear main comm port
	*reg_mem = 0x2A01;

	// Wait for the sub-CPU to be running
	wait_cpu_running();

	// Enable level 2 interrupts on sub-CPU to poke it during vblank
	*reg_cpu = (*reg_cpu & 0xEFFF) | SEGACD_CPU_IEN_MASK;
	const uint8_t int_sav = md_sys_di();

	// Wait for the sub-CPU to set sub comm port to indicate it is
	// alive and working correctly
	while (comm_read() != 'I')
	{
		// Show cool junk while we wait
		static int32_t timeout = 0;
		timeout++;

		if (timeout > 2000000)
		{
			return;
		}
	}

	// Wait for sub-CPU being ready to receive commands
	while (comm_read() != 0x00)
	{
		__asm__("nop");
	}

	if (int_sav) md_sys_ei();
}

int16_t cdaudio_init(void)
{
	(void)reg_hint;
	(void)reg_stopwatch;
	segacd_bios_addr = check_hardware();
	if (!segacd_bios_addr)
	{
		return 0;
	}
	reset_gate_array();
	subcpu_setup();
	if (!decompress_bios())
	{
		segacd_bios_addr = 0;
		return 0;
	}
	start_subcpu();
	return 0;
	cdaudio_check_disc();

	get_disc_info();
	wait_for_ready();
	return 1;
}

static inline uint16_t cdaudio_playtrack(uint16_t trk, uint8_t mode)
{
	cdaudio_check_disc();
	get_disc_info();

	if (cdaudio_get_status() == SEGACD_STAT_NODISC ||
	    cdaudio_get_status() == SEGACD_STAT_OPEN ||
		trk > info.last_tno ||
		trk < info.first_tno)
	{
		return 0;
	}

	*track_w = trk;
	*pmode_w = mode;
	wait_do_cmd('P');
	wait_cmd_ack();
	comm_write(SEGACD_CMD_ACK);
	return 1;
}

/* ----- User functions ----- */

int16_t cdaudio_play_once(uint16_t trk)
{
	return cdaudio_playtrack(trk, SEGACD_PLAYMODE_ONCE);
}

int16_t cdaudio_play_loop(uint16_t trk)
{
	return cdaudio_playtrack(trk, SEGACD_PLAYMODE_LOOP);
}

void cdaudio_stop(void)
{
	if (cdaudio_is_active())
	{
		wait_do_cmd('S');
		wait_cmd_ack();
		comm_write(SEGACD_CMD_ACK);
	}
}

void cdaudio_pause(void)
{
	get_disc_info();

	if (cdaudio_get_status() == SEGACD_STAT_PLAYING)
	{
		wait_do_cmd('Z');
		wait_cmd_ack();
		comm_write(SEGACD_CMD_ACK);
	}
}

void cdaudio_resume(void)
{
	get_disc_info();

	if (cdaudio_get_status() == SEGACD_STAT_PAUSED)
	{
		wait_do_cmd('Z');
		wait_cmd_ack();
		comm_write(SEGACD_CMD_ACK);
	}
}

inline int16_t cdaudio_is_active(void)
{
	return (segacd_bios_addr) ? 1 : 0;
}
