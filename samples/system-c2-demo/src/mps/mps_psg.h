#ifndef MPS_PSG_H
#define MPS_PSG_H

// TODO: Replace this; we should just use the md-framework PSG code.

#define PSG_NOISE_PERIODIC 0
#define PSG_NOISE_WHITE 1

static const uint16_t psg_periods[] = 
{
	0,
	4 * 0x194,
	4 * 0x17D,
	4 * 0x168,
	4 * 0x153,
	4 * 0x140,
	4 * 0x12E,
	4 * 0x11D,
	4 * 0x10D,
	4 * 0x0FE,
	4 * 0x0F0,
	4 * 0x0E2,
	4 * 0x0D6
};

// TODO: Literally any of this
static inline uint16_t psg_map_note_period(uint8_t note, uint8_t octave)
{
	return (psg_periods[note]) >> octave;
}

// These commit to SN76476.
static inline void psg_set_att(uint8_t idx, uint8_t att)
{
	volatile uint8_t *sn = (volatile uint8_t *)0xC00011;
	*sn = 0x90 | ((idx & 0x03) << 5) | (att & 0x0F);
}

static inline void psg_set_period(uint8_t idx, uint16_t period)
{
	volatile uint8_t *sn = (volatile uint8_t *)0xC00011;
	*sn = 0x80 | ((idx & 0x03) << 5) | (period & 0x0F);
	*sn = (period >> 4) & 0x3F;
}

static inline void psg_set_noise(uint8_t type, uint8_t rate)
{
	volatile uint8_t *sn = (volatile uint8_t *)0xC00011;
	*sn = 0xE0 | (rate << 1) | type;
	
}
#endif // MPS_PSG_H
