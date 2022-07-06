#ifndef MPS_FM_H
#define MPS_FM_H

#include "md/megadrive.h"
#include "mps/mps_types.h"

static inline uint8_t fm_gen_freq_hi(uint8_t note, uint8_t octave);
static inline uint8_t fm_gen_freq_lo(uint8_t note);
static inline uint16_t fm_map_note_freq(uint8_t note);
static inline void fm_set_instrument(uint8_t idx, MpsInstrument *instrument);
static inline void fm_set_note(uint8_t idx, uint8_t note, uint8_t octave);
static inline void fm_set_freq(uint8_t idx, uint16_t freq, uint8_t octave);
static inline void fm_set_panning(uint8_t idx, uint8_t value, MpsInstrument *instrument);
static inline void fm_set_tl(uint8_t idx, uint8_t op, uint8_t val);
static inline void fm_keyoff(uint8_t idx);
static inline void fm_keyon(uint8_t idx);
static inline void fm_set_dac_en(uint8_t enabled);
static inline void fm_set_lfo(uint8_t enabled, uint8_t freq);

static const uint16_t note_freq[] =
{
	644, // Base C, unused
	682, // C#
	723, // D
	766, // D#
	811, // E
	859, // F
	910, // F#
	965, // G
	1022, // G#
	1083, // A
	1147, // A#
	1215, // B
	1288 // C
};

static inline uint8_t fm_gen_freq_hi(uint8_t note, uint8_t octave)
{
	return (((note_freq[note]) >> 8) & 0x07) | (octave << 3);
}

static inline uint8_t fm_gen_freq_lo(uint8_t note)
{
	return (note_freq[note]) & 0x00FF;
}

static inline uint16_t fm_map_note_freq(uint8_t note)
{
	return note_freq[note];
}

static inline void fm_set_instrument(uint8_t idx, MpsInstrument *instrument)
{
	uint8_t chan_off = idx;
	uint8_t part = (idx >= 3 ? 1 : 0);;
	if (chan_off >= 3)
	{
		chan_off -= 3;
	}

	for (uint8_t i = 0; i < 4; i++)
	{
		md_opn_write(part, 0x30 + chan_off + (4 * i),
		             instrument->fm.r30_dt_mul[i]);
		// TL is handld with set_tl calls
		md_opn_write(part, 0x50 + chan_off + (4 * i),
		             instrument->fm.r50_rs_ar[i]);
		md_opn_write(part, 0x60 + chan_off + (4 * i),
		             instrument->fm.r60_am_d1r[i]);
		md_opn_write(part, 0x70 + chan_off + (4 * i),
		             instrument->fm.r70_d2r[i]);
		md_opn_write(part, 0x80 + chan_off + (4 * i),
		             instrument->fm.r80_d1l_rr[i]);
		md_opn_write(part, 0x90 + chan_off + (4 * i),
		             instrument->fm.r90_ssg_eg[i]);
	}
	md_opn_write(part, 0xB0 + chan_off,
	             instrument->fm.rb0_fb_alg);
	md_opn_write(part, 0xB4 + chan_off,
	             instrument->fm.rb4_stereo_lfo);
}

static inline void fm_set_tl(uint8_t idx, uint8_t op, uint8_t val)
{
	uint8_t chan_off = idx;
	uint8_t part = (idx >= 3 ? 1 : 0);
	if (chan_off >= 3)
	{
		chan_off -= 3;
	}
	md_opn_write(part, 0x40 + chan_off + (4 * op), val);
}

static inline void fm_set_freq(uint8_t idx, uint16_t freq, uint8_t octave)
{
	uint8_t chan_off = idx;
	uint8_t part = (idx >= 3 ? 1 : 0);
	if (chan_off >= 3)
	{
		chan_off -= 3;
	}

	uint8_t hibyte = ((freq >> 8) & 0x07) | (octave << 3);
	uint8_t lobyte = freq & 0x00FF;

	md_opn_write(part, 0xA4 + chan_off, hibyte);
	md_opn_write(part, 0xA0 + chan_off, lobyte);
}

static inline void fm_set_note(uint8_t idx, uint8_t note, uint8_t octave)
{
	uint8_t chan_off = idx;
	uint8_t part = (idx >= 3 ? 1 : 0);;
	if (chan_off >= 3)
	{
		chan_off -= 3;
	}

	md_opn_write(part, 0xA4 + chan_off, fm_gen_freq_hi(note, octave));
	md_opn_write(part, 0xA0 + chan_off, fm_gen_freq_lo(note));
}

static inline void fm_set_panning(uint8_t idx, uint8_t value, MpsInstrument *instrument)
{
	uint8_t chan_off = idx;
	uint8_t part = (idx >= 3 ? 1 : 0);;
	if (chan_off >= 3)
	{
		chan_off -= 3;
	}

	uint8_t wval = instrument->fm.rb4_stereo_lfo | value;
	md_opn_write(part, 0xB4 + chan_off, wval);
}

// Lookup for keyon/keyoff indeces
static uint8_t key_table[] =
{
	0x00,
	0x01,
	0x02,
	0x04,
	0x05,
	0x06,
};

static inline void fm_keyoff(uint8_t idx)
{
	md_opn_write(0, 0x28, key_table[idx]);
}

static inline void fm_keyon(uint8_t idx)
{
	md_opn_write(0, 0x28, 0xF0 | key_table[idx]);
}

static inline void fm_set_dac_en(uint8_t enabled)
{
	md_opn_write(0, 0x2B, enabled ? 0x80 : 0);
}

static inline void fm_set_lfo(uint8_t enabled, uint8_t freq)
{
	md_opn_write(0, 0x22, (enabled ? 0x08 : 0x00) | freq);
}

#endif // MPS_FM_H
