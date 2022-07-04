/* md-toolchain PSG audio support
Michael Moffitt 2018-2020 */
#ifndef MD_PSG_H
#define MD_PSG_H

// TODO: PSG noise control

// Base note frequencies - tuned to 4th octave
#define PSG_BASE_C 0x1AC
#define PSG_BASE_Db 0x194
#define PSG_BASE_D 0x17D
#define PSG_BASE_Eb 0x168
#define PSG_BASE_E 0x153
#define PSG_BASE_F 0x140
#define PSG_BASE_Gb 0x12E
#define PSG_BASE_G 0x11D
#define PSG_BASE_Ab 0x10D
#define PSG_BASE_A 0x0FE
#define PSG_BASE_Bb 0x0F0
#define PSG_BASE_B 0x0E2

#define PSG_NOTE_Db 1
#define PSG_NOTE_D  2
#define PSG_NOTE_Eb 3
#define PSG_NOTE_E  4
#define PSG_NOTE_F  5
#define PSG_NOTE_Gb 6
#define PSG_NOTE_G  7
#define PSG_NOTE_Ab 8
#define PSG_NOTE_A  9
#define PSG_NOTE_Bb 10
#define PSG_NOTE_B  11
#define PSG_NOTE_C  12

#define PSG_PORT *(volatile uint8_t *)0xC00011

// Megadrive PSG functions
static inline void md_psg_vol(uint8_t chan, uint8_t vol);
static inline void md_psg_pitch(uint8_t chan, uint16_t pitch);
static inline void md_psg_tone(uint8_t chan, uint8_t vol, uint16_t pitch);
static inline void md_psg_note(uint8_t chan, uint8_t note, uint8_t octave);


static inline void md_psg_vol(uint8_t chan, uint8_t vol)
{
	PSG_PORT = 0x90 | ((chan & 0x03) << 5) | (vol & 0x0F);
}

static inline void md_psg_pitch(uint8_t chan, uint16_t pitch)
{
	PSG_PORT = 0x80 | ((chan & 0x03) << 5) | (pitch & 0x0F);
	PSG_PORT = (pitch >> 4) & 0x3F;
}

static inline void md_psg_tone(uint8_t chan, uint8_t vol, uint16_t pitch)
{	
	md_psg_pitch(chan,pitch);
	md_psg_vol(chan,vol);
}

static inline uint32_t note_lookup(uint8_t note)
{
	switch (note)
	{
		case PSG_NOTE_C:
			return PSG_BASE_C;
		case PSG_NOTE_Db:
			return PSG_BASE_Db;
		case PSG_NOTE_D:
			return PSG_BASE_D;
		case PSG_NOTE_Eb:
			return PSG_BASE_Eb;
		case PSG_NOTE_E:
			return PSG_BASE_E;
		case PSG_NOTE_F:
			return PSG_BASE_F;
		case PSG_NOTE_Gb:
			return PSG_BASE_Gb;
		case PSG_NOTE_G:
			return PSG_BASE_G;
		case PSG_NOTE_Ab:
			return PSG_BASE_Ab;
		case PSG_NOTE_A:
			return PSG_BASE_A;
		case PSG_NOTE_Bb:
			return PSG_BASE_Bb;
		case PSG_NOTE_B:
			return PSG_BASE_B;
	}
	return PSG_BASE_C;
}

static inline void md_psg_note(uint8_t chan, uint8_t note, uint8_t octave)
{
	uint32_t base = note_lookup(note);
	base = base << 1;
	base = base >> octave;
	md_psg_pitch(chan,base);
}

#endif  // MD_PAL_H
