// Sega CD Mode 1 support code for SGDK
// Michael Moffitt (https://github.com/mikejmoffitt)
// Adapted from Chilly Willy's Mode 1 CD Player

#ifndef CDAUDIO_H
#define CDAUDIO_H

#define SEGACD_REG_CPU 0xA12000
#define SEGACD_REG_MEM 0xA12002
#define SEGACD_REG_HINT 0xA12006
#define SEGACD_REG_STOPWATCH 0xA1200C
#define SEGACD_REG_COMM 0xA1200E

#define SEGACD_TRACK_W 0xA12010
#define SEGACD_PMODE_W 0xA12012

#define SEGACD_PROGRAM_ADDR 0x420000
#define SEGACD_PROGRAM_OFF 0x426000
#define SEGACD_PROGRAM_LEN 0x20000

#define SEGACD_CPU_IEN_MASK 0x8000

#define SEGACD_BIOS_STAT 0xA12020
#define SEGACD_FIRST_TNO 0xA12022
#define SEGACD_LAST_TNO 0xA12023
#define SEGACD_DRV_VERS 0xA12024
#define SEGACD_FLAG 0xA12025

#define SCD_OFFSET 0x6D

#define SCD_BIOSLOC_1 0x415800
#define SCD_BIOSLOC_2 0x416000
#define SCD_BIOSLOC_3 0x41AD00

#define SEGACD_CMD_ACK 0x00

#define SEGACD_PLAYMODE_ONCE 0x00
#define SEGACD_PLAYMODE_LOOP 0x01

#define SEGACD_STAT_STOPPED 0
#define SEGACD_STAT_PLAYING 1
#define SEGACD_STAT_SCANNING 3
#define SEGACD_STAT_PAUSED 5
#define SEGACD_STAT_SEEKING 8
#define SEGACD_STAT_NODISC 16
#define SEGACD_STAT_READINGTOC 32
#define SEGACD_STAT_OPEN 64

#define DBCOL(c) VDP_setPaletteColor(0,c)

#include <stdint.h>

extern volatile uint8_t *segacd_bios_addr;

int16_t cdaudio_init(void);

// Playback functions. Returns non-zero when the play command worked.
// Will return zero if there is no disc, or the tray is open.
int16_t cdaudio_play_once(uint16_t trk);
int16_t cdaudio_play_loop(uint16_t trk);

// Stop playing. If there is nothing playing it is a no-op.
void cdaudio_stop(void);

// Pause / restart.
void cdaudio_pause(void);
void cdaudio_resume(void);
int16_t cdaudio_is_active(void);

#endif
