#ifndef MPS_FX_H
#define MPS_FX_H

#include "mps/mps_play.h"
#include "mps/mps_types.h"
#include "mps/mps_fm.h"

// Basic effects
#define MPS_FX_SWEEP_UP 0x01
#define MPS_FX_SWEEP_DOWN 0x02
#define MPS_FX_SWEEP_PORTA 0x03
#define MPS_FX_VIBRATO 0x04
#define MPS_FX_PAN 0x08
#define MPS_FX_JUMP 0x0B
// YM2612-specific effects
#define MPS_FX_LFO 0x10
#define MPS_FX_FB 0x11
#define MPS_FX_TL1 0x12
#define MPS_FX_TL2 0x13
#define MPS_FX_TL3 0x14
#define MPS_FX_TL4 0x15
#define MPS_FX_MUL 0x16
#define MPS_FX_DAC 0x17
#define MPS_FX_ATK 0x19
#define MPS_FX_ATK1 0x1A
#define MPS_FX_ATK2 0x1B
#define MPS_FX_ATK3 0x1C
#define MPS_FX_ATK4 0x1D
#define MPS_FX_NOISE 0x20

extern MpsState mps_state;

static inline void mps_handle_fx(MpsChannelState *chan, MpsCell *cell);

// Run when an effect column entry is encountered
static inline void mps_handle_fx(MpsChannelState *chan, MpsCell *cell)
{
	switch(cell->effect_type)
	{
		default:
			break;
		case MPS_FX_SWEEP_UP:
			chan->sweep_mode = MPS_SWEEP_MODE_UP;
			chan->sweep_speed = cell->effect_value;
			break;
		case MPS_FX_SWEEP_DOWN:
			chan->sweep_mode = MPS_SWEEP_MODE_DOWN;
			chan->sweep_speed = cell->effect_value;
			break;
		case MPS_FX_SWEEP_PORTA:
			chan->sweep_mode = MPS_SWEEP_MODE_PORTA;
			chan->sweep_speed = cell->effect_value;
			break;
		case MPS_FX_VIBRATO:
			chan->vib_depth = VIB_SPEED_MOD * (cell->effect_value & 0x0F);
			chan->vib_speed = ((cell->effect_value & 0xF0) >> 4);
			break;
		case MPS_FX_PAN:
			if (chan->num < 6)
			{
				uint8_t panval = 0;
				if (cell->effect_value & 0x01)
				{
					panval |= MPS_PAN_RIGHT;
				}
				if (cell->effect_value & 0x10)
				{
					panval |= MPS_PAN_LEFT;
				}
				chan->panning = panval;
				z80_busreq_hold();
				fm_set_panning(chan->num, panval, chan->instrument);
				z80_busreq_release();
			}
			break;
		case MPS_FX_JUMP:
			// TODO: Should store a next-pattern-index and only use it once
			// tick_cnt is zero. Otherwise jumps will occur too quickly.
			// TODO TODO: Maybe not, since it won't reset tick_cnt, the cadence
			// of the track won't actualy be affected? Play with this. Might be
			// fine as-is.
			chan->pattern_idx = cell->effect_value;
			chan->current_pattern = (MpsCell *)((uint8_t *)mps_state.track + chan->arrangement[chan->pattern_idx]);
			chan->row_idx = 0;
			
			break;
		case MPS_FX_LFO:
			z80_busreq_hold();
			fm_set_lfo(cell->effect_value & 0xF0, cell->effect_value & 0x07);
			z80_busreq_release();
			break;
		case MPS_FX_FB:
			if (chan->num < 6)
			{
				uint8_t chan_off = chan->num;
				uint8_t part = (chan_off >= 3 ? 1 : 0);;
				if (chan_off >= 3)
				{
					chan_off -= 3;
				}
				z80_busreq_hold();
				ym2612_write(part, 0xB0 + chan_off,
				             (chan->instrument->fm.rb0_fb_alg & 0xC7) | ((cell->effect_value & 0x07) << 3));
				z80_busreq_release();
			}
			break;
		case MPS_FX_DAC:
			mps_state.dac_en = cell->effect_value;
			z80_busreq_hold();
			fm_set_dac_en(mps_state.dac_en);
			z80_busreq_release();
			break;
	}
}



#endif
