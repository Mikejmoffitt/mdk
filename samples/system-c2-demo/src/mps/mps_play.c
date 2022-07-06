#include "mps/mps_play.h"
#include "mps/mps_freq.h"
#include "mps/mps_fm.h"
#include "mps/mps_psg.h"
#include "md/megadrive.h"
#include "util/text.h"
#include <stdlib.h>

#define MPS_ARR_OFFSET 10
#define MPS_ARR_SEEK(state_, i_) (2 * i_ * state_.track->track_length)

#define MPS_INST_OFFSET(state_) (MPS_ARR_OFFSET +\
                        (2 * (MPS_CHANCNT) * state_.track->track_length))

static MpsState mps_state;


// Lookup for debug print
static const char *note_names[] =
{
	"--",
	"C#",
	"D-",
	"D#",
	"E-",
	"F-",
	"F#",
	"G-",
	"G#",
	"A-",
	"A#",
	"B-",
	"C-"
};

static void mps_print_cell(uint8_t y, MpsCell *cell)
{
	// note & octave
	if (cell->note == MPS_NOTE_OFF)
	{
		text_puts(VDP_PLANE_A, 0, y, " === ");
	}
	else if (cell->note >= 1 && cell->note <= 12)
	{
		char buffer[5];
		buffer[0] = ' ';
		buffer[1] = note_names[cell->note][0];
		buffer[2] = note_names[cell->note][1];
		buffer[3] = '0' + cell->octave;
		buffer[4] = '\0';
		text_puts(VDP_PLANE_A, 0, y, buffer);
	}
	else
	{
		text_puts(VDP_PLANE_A, 0, y, " --- ");
	}
	// volume
	if (cell->volume != MPS_VOL_EMPTY && cell->volume <= 0x7F)
	{
		char buffer[3];
		uint8_t test_val = cell->volume;
		for (uint16_t i = 0; i < 2; i++)
		{
			const uint8_t nybble = (test_val & 0xF0) >> 4;
			if (nybble < 0xA) buffer[i] = '0' + nybble;
			else buffer[i] = 'A' + nybble;
			test_val = test_val << 4;
		}
		buffer[2] = '\0';
		text_puts(VDP_PLANE_A, 5, y, buffer);
	}
	else
	{
		text_puts(VDP_PLANE_A, 5, y, "-- ");
	}
	// instrument
	if (cell->note >= 1 && cell->note <= 12)
	{
		char buffer[3];
		uint8_t test_val = cell->instrument;
		for (uint16_t i = 0; i < 2; i++)
		{
			const uint8_t nybble = (test_val & 0xF0) >> 4;
			if (nybble < 0xA) buffer[i] = '0' + nybble;
			else buffer[i] = 'A' + nybble;
			test_val = test_val << 4;
		}
		buffer[2] = '\0';
		text_puts(VDP_PLANE_A, 8, y, buffer);
	}
	else
	{
		text_puts(VDP_PLANE_A, 8, y, "-- ");
	}
	// effect
	if (cell->effect_type != MPS_EFFECT_EMPTY)
	{
		char buffer[5];
		uint8_t test_val = cell->effect_type;
		for (uint16_t i = 0; i < 2; i++)
		{
			const uint8_t nybble = (test_val & 0xF0) >> 4;
			if (nybble < 0xA) buffer[i] = '0' + nybble;
			else buffer[i] = 'A' + nybble;
			test_val = test_val << 4;
		}
		test_val = cell->effect_value;
		for (uint16_t i = 0; i < 2; i++)
		{
			const uint8_t nybble = (test_val & 0xF0) >> 4;
			if (nybble < 0xA) buffer[i+2] = '0' + nybble;
			else buffer[i+2] = 'A' + nybble;
			test_val = test_val << 4;
		}
		buffer[4] = '\0';
		text_puts(VDP_PLANE_A, 12, y, buffer);
	}
	else
	{
		text_puts(VDP_PLANE_A, 12, y, "----\n");
	}
}


/*

current_pattern points to the first cell in the present pattern.
A row is a division of a pattern. there are pattern_length rows in a pattern.
A tick is a division of a row. There are (time base * tick) ticks per row.

*/


static inline uint8_t tl_reduce(uint8_t tl_orig, uint8_t volume)
{
	uint16_t vol_orig = 0x7F - tl_orig;
	uint16_t scaled = (((uint16_t)volume + 1) * (vol_orig + 1)) >> 7;
	return 0x7F - scaled;
}

// Modifies TL of carriers of current instrument and writes new TL to YM2612
// !! Need busreq first.
static inline void mps_set_chan_tl(MpsChannelState *chan)
{
	uint8_t algo = chan->instrument->fm.rb0_fb_alg & 0x07;
	uint8_t tl[4];
	tl[0] = chan->instrument->fm.r40_tl[0];
	tl[1] = chan->instrument->fm.r40_tl[1];
	tl[2] = chan->instrument->fm.r40_tl[2];
	tl[3] = chan->instrument->fm.r40_tl[3];

	if (chan->volume == 0x7F)
	{
		goto tl_write;
	}

	// Always change op 4
	tl[3] = tl_reduce(tl[3], chan->volume);

	// Change op 2 if 4 or above
	if (algo >= 4)
	{
		tl[2] = tl_reduce(tl[2], chan->volume);
	}
	// Change op 3 if 5 or above
	if (algo >= 5)
	{
		tl[1] = tl_reduce(tl[1], chan->volume);
	}

	// Change op 1 if 7
	if (algo == 7)
	{
		tl[0] = tl_reduce(tl[0], chan->volume);
	}

tl_write:
	fm_set_tl(chan->num, 2, tl[2]);
	fm_set_tl(chan->num, 0, tl[0]);
	fm_set_tl(chan->num, 1, tl[1]);
	fm_set_tl(chan->num, 3, tl[3]);
}

// Set PSG attenuation based on volume setting
static inline void mps_set_chan_att(MpsChannelState *chan)
{
	if (chan->key_state == KEY_ON)
	{
		uint8_t vol_orig = chan->instrument->psg.envelope[chan->psg_env_index] & ENV_ATT_MASK;
		uint16_t scaled = ((vol_orig+1) * (chan->volume+1)) >> 4;
		psg_set_att(chan->num - 6, 0xF - scaled);
	}
}

static void mps_reset_channels(void)
{
	md_sys_z80_bus_req();
	for (uint8_t i = 0; i < MPS_CHANCNT; i++)
	{
		MpsChannelState *chan = &(mps_state.channels[i]);

		chan->num = i;

		chan->arrangement = (uint16_t *)((uint8_t *)mps_state.track + MPS_ARR_OFFSET + (2 * chan->num * mps_state.track->track_length));
		chan->pattern_idx = 0;

		chan->current_pattern = (MpsCell *)((uint8_t *)mps_state.track + chan->arrangement[chan->pattern_idx]);
		chan->row_idx = 0;
		chan->tick_delay = 0;

//		chan->note = MPS_NOTE_EMPTY;
		chan->octave = 0;
		chan->target_octave = 0;
		chan->volume = 0x7F;
		chan->instrument = (MpsInstrument *)((uint8_t *)mps_state.track + MPS_INST_OFFSET(mps_state));
		chan->panning = MPS_PAN_BOTH;
		chan->nops_count = 0;
		chan->prev_instrument_id = 0xFF;

		chan->key_state = KEY_OFF;
		chan->psg_env_index = 0;
		chan->psg_env_loop = PSG_LOOP_NONE;

		chan->vib_depth = 0;
		chan->vib_speed = 0;
		chan->vib_phase = VIB_PHASE_UP;
		chan->vib_offset = 0;

		chan->sweep_mode = MPS_SWEEP_MODE_OFF;
		chan->sweep_speed = 0;
		chan->frequency = 0;
		chan->target_frequency = 0;

		if (i < 6)
		{
			fm_keyoff(i);
		}

	}
	fm_set_dac_en(0);
//	fm_set_lfo(0, 0);

	// Default PSG mode is noise
	psg_set_noise(PSG_NOISE_WHITE, 3);
	md_sys_z80_bus_release();
}

// Set up music data for playback
void mps_init(const MpsHeader *track)
{
	mps_state.status = MPS_STATUS_IDLE;
	mps_state.track = (MpsHeader *)track;
	mps_state.instruments = (MpsInstrument *)((uint8_t *)track + MPS_INST_OFFSET(mps_state));
	mps_state.dac_en = 0;
	if (track->mps_name[0] != 'M' && track->mps_name[1] != 'P' &&
	    track->mps_name[2] != 'S')
	{
		mps_state.track = NULL;
	}
	mps_reset_channels();
}

// Start playing music
void mps_play(void)
{
	if (mps_state.track != NULL)
	{
		mps_state.status = MPS_STATUS_PLAY;
	}
}

// Stop playing music
void mps_stop(void)
{
	mps_state.status = MPS_STATUS_IDLE;
}

static inline void mps_handle_note(MpsChannelState *chan, MpsCell *cell)
{
	if (cell->note == MPS_NOTE_OFF)
	{
		chan->key_state = KEY_OFF;
		if (chan->num < 6)
		{
			md_sys_z80_bus_req();
			fm_keyoff(chan->num);
			if (cell->note == MPS_NOTE_CUT)
			{
				uint8_t vol_store = chan->volume;
				chan->volume = 0;
				mps_set_chan_tl(chan);
				chan->volume = vol_store;
			}
			md_sys_z80_bus_release();
		}
		else
		{
			psg_set_att(chan->num - 6, 0xF);
		}
	}
	else if (cell->note != MPS_NOTE_EMPTY)
	{
//		chan->note = cell->note;
		// Octave is separate as it must be target in FM

		uint8_t *inst_base = (uint8_t *)mps_state.instruments;

		chan->key_state = KEY_ON;
		if (chan->num < 6)
		{
			chan->target_frequency = fm_map_note_freq(cell->note);
			chan->target_octave = cell->octave;
			md_sys_z80_bus_req();
			if (chan->prev_instrument_id != cell->instrument && cell->instrument != MPS_INSTRUMENT_EMPTY)
			{
				chan->instrument = (MpsInstrument *)(inst_base + (cell->instrument << 5));
				fm_set_instrument(chan->num, chan->instrument);
				chan->prev_instrument_id = cell->instrument;
			}
			fm_set_panning(chan->num, chan->panning, chan->instrument);
			mps_set_chan_tl(chan);
			if (cell->effect_type != MPS_FX_SWEEP_PORTA)
			{
				chan->frequency = chan->target_frequency;
				chan->octave = chan->target_octave;
				fm_keyoff(chan->num);
			}
//			fm_set_note(chan->num, cell->note, cell->octave);
			fm_keyon(chan->num);
			md_sys_z80_bus_release();
		}
		else
		{
			chan->octave = cell->octave;
			if (cell->instrument != MPS_INSTRUMENT_EMPTY)
			{
				chan->instrument = (MpsInstrument *)(inst_base + (cell->instrument << 5));
				chan->prev_instrument_id = cell->instrument;
			}
			chan->target_frequency = psg_map_note_period(cell->note, cell->octave);

			if (cell->effect_type != MPS_FX_SWEEP_PORTA)
			{
				chan->psg_env_index = 0;
				chan->psg_env_loop = PSG_LOOP_NONE;
				chan->frequency = chan->target_frequency;
				// TODO: Generate PSG period based on note and octave
			}

			// Retrigger envelope
		}
	}
}

// Run when a volume column entry is encountered
static inline void mps_handle_vol(MpsChannelState *chan, MpsCell *cell)
{
	chan->volume = cell->volume;

	if (chan->num < 6)
	{
		md_sys_z80_bus_req();
		mps_set_chan_tl(chan);
		md_sys_z80_bus_release();
	}
	// PSG changes do not need to be done here; they will already be handled by
	// the envelope generator.
}

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
			if (chan->num < 6)
			{
				chan->vib_depth = VIB_SPEED_MOD * (cell->effect_value & 0x0F);
				chan->vib_speed = (cell->effect_value & 0xF0) >> 4;
			}
			else
			{
				chan->vib_depth = VIB_SPEED_MOD * (cell->effect_value & 0x0F);
				chan->vib_speed = ((cell->effect_value & 0xF0) >> 4);
			}
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
				md_sys_z80_bus_req();
				fm_set_panning(chan->num, panval, chan->instrument);
				md_sys_z80_bus_release();
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
		case MPS_FX_DAC:
			mps_state.dac_en = cell->effect_value;
			md_sys_z80_bus_req();
			fm_set_dac_en(mps_state.dac_en);
			md_sys_z80_bus_release();
			break;
	}
}

// move forward one row, play a note, NOP, handle volume or effects, etc
static inline void mps_handle_cell(MpsChannelState *chan, MpsCell *cell)
{
	mps_print_cell(chan->num, cell);
	uint8_t tick = mps_state.track->tick[chan->row_idx & 0x01] - 1;

	// Handle an extended delay instead of a note
	if (chan->nops_count == 0)
	{
		if (cell->note == MPS_NOTE_NOP)
		{
			// cell->octave is used as the NOP argument
			chan->nops_count = cell->octave - 1;
		}
		// Handle a normal note
		else
		{
			// Sweep is reset on every note
			chan->sweep_mode = MPS_SWEEP_MODE_OFF;

			mps_handle_note(chan, cell);
			if (cell->volume != MPS_VOL_EMPTY)
			{
				mps_handle_vol(chan, cell);
			}


			if (cell->effect_type != MPS_EFFECT_EMPTY)
			{
				mps_handle_fx(chan, cell);
			}
		}
		chan->current_pattern = (MpsCell *)((uint8_t *)chan->current_pattern + sizeof(MpsCell));
	}
	else
	{
		chan->nops_count--;
	}
	chan->tick_delay = tick * (mps_state.track->time_base+1);
	chan->row_idx++;
}

/*

Channel tick:

	Tick effects and envelope (PSG)

	Run down tick delay, or continue
	Check new cell type and handle cell
		If nop:
			Set tick delay to (time_base * tick) * 'octave'
			Increment row idx by 'octave'
		Else If key off:
			Set key off register
			Set tick delay to (time_base * tick)
			Increment row_idx
		Else If note:
			If note value is non-empty:
				Set instrument registers
				Set frequency
				Set key on register
			Set tick delay to (time_base * tick)
			Increment row_idx
		Increment current_pattern by sizeof(MpsCell)

	If row_idx >= pattern_length
		Increment pattern_idx by 1
		if pattern_idx >= track_length
			Set pattern_idx to zero
		Set current_pattern to arrangement[pattern_idx]

*/

static inline void mps_psg_run_envelope(MpsChannelState *chan)
{
	if (chan->key_state == KEY_ON)
	{
		uint8_t env = chan->instrument->psg.envelope[chan->psg_env_index];

		// Mark loop point if found
		if (env & ENV_PROP_MASK & ENV_PROP_LOOP)
		{
			chan->psg_env_loop = chan->psg_env_index;
		}

		// Check for end, and loop if needed
		if (env & ENV_PROP_MASK & ENV_PROP_END)
		{
			if (chan->psg_env_loop != PSG_LOOP_NONE &&
			    chan->psg_env_loop < chan->psg_env_index)
			{
				chan->psg_env_index = chan->psg_env_loop;
			}
		}
		else
		{
			chan->psg_env_index++;
		}
	}
}

static inline void mps_chan_tick(MpsChannelState *chan)
{
	// Run one tick of frequency modulation (vibrato, sweeps)
	mps_run_freq_mod(chan);

	if (chan->tick_delay > 0)
	{
		chan->tick_delay--;
		goto freq_commit;
	}

	// check if delay or note
	MpsCell *cell = (chan->current_pattern);
	mps_handle_cell(chan, cell);

	if (chan->row_idx >= mps_state.track->pattern_length)
	{
		chan->row_idx = 0;
		chan->pattern_idx += 1;
		if (chan->pattern_idx >= mps_state.track->track_length)
		{
			chan->pattern_idx = 0;
		}
		chan->current_pattern = (MpsCell *)((uint8_t *)mps_state.track + chan->arrangement[chan->pattern_idx]);
	}
freq_commit:
	if (chan->num < 6)
	{
		md_sys_z80_bus_req();
		fm_set_freq(chan->num, chan->frequency + chan->vib_offset, chan->octave);
		md_sys_z80_bus_release();
	}
	else
	{
		// Vibrato relationship is reversed as PSG operates on period.
		psg_set_period(chan->num-6, chan->frequency - (chan->vib_offset >> VIB_PSG_DIV));

		
		// Manual PSG attenuation envelope
		if (chan->num >= 6)
		{
			mps_psg_run_envelope(chan);
		}
		// Commit attenuation level
		mps_set_chan_att(chan);
	}

}

// Run one frame of music playback. Usually done during vblank.
void mps_tick(void)
{
	if (mps_state.status != MPS_STATUS_PLAY) return;
	MpsChannelState *chan = &mps_state.channels[0];
	for (uint8_t i = 0; i < MPS_CHANCNT; i++)
	{
		mps_chan_tick(chan);
		chan++;
	}
}

// Get metadata about the track.
MpsHeader *mps_get_header(void)
{
	return mps_state.track;
}

// Get channel state
MpsChannelState *mps_get_chanstate(uint8_t channel)
{
	return &(mps_state.channels[channel]);
}
