#ifndef MPS_FREQ_H
#define MPS_FREQ_H

#include "mps/mps_types.h"

static inline void mps_main_sweep_logic(uint16_t *freq_in, uint16_t freq_target, uint8_t speed)
{
	if (*freq_in > freq_target)
	{
		*freq_in -= speed;
		// Prevent oscillations
		if (*freq_in < freq_target)
		{
			*freq_in = freq_target;
		}
	}
	else if (*freq_in < freq_target)
	{
		*freq_in += speed;
		// Prevent oscillations
		if (*freq_in > freq_target)
		{
			*freq_in = freq_target;
		}
	}
}

static inline void mps_fm_sweep(uint16_t *freq_in, uint8_t *oct_in, uint16_t freq_target, uint8_t oct_target, uint8_t speed)
{
	if (*oct_in == oct_target)
	{
		mps_main_sweep_logic(freq_in, freq_target, speed);
	}
	else if (*oct_in > oct_target)
	{
		if (*freq_in > speed)
		{
			*freq_in -= speed;
		}
		else
		{
			*oct_in -= 1;
			*freq_in = *freq_in << 1;
		}
	}
	else
	{
		if (*freq_in < (0xFFFF - speed))
		{
			*freq_in += speed;
		}
		else
		{
			*oct_in += 1;
			*freq_in = *freq_in >> 1;
		}
	}
}

static inline void mps_run_freq_mod(MpsChannelState *chan)
{
	// Vibrato
	if (chan->vib_depth != 0)
	{
		if (chan->vib_phase == VIB_PHASE_DOWN)
		{
			chan->vib_offset += chan->vib_speed;
			if (chan->vib_offset >= chan->vib_depth)
			{
				chan->vib_phase = VIB_PHASE_UP;
			}
		}
		else
		{
			chan->vib_offset -= chan->vib_speed;
			if (chan->vib_offset <= chan->vib_depth * -1)
			{
				chan->vib_phase = VIB_PHASE_DOWN;
			}
		}
	}
	else
	{
		chan->vib_offset = 0;
	}

	// Sweeps up/down

	// For PSG, sweep direction is inverse, since period is specified, rather
	// than frequency.
	if (chan->sweep_mode == MPS_SWEEP_MODE_PORTA)
	{
		if (chan->num < 6)
		{
			mps_fm_sweep(&chan->frequency, &chan->octave,
			             chan->target_frequency, chan->target_octave,
			             chan->sweep_speed);
		}
		else
		{
			mps_main_sweep_logic(&chan->frequency, chan->target_frequency,
			                     chan->sweep_speed);
		}
	}
	else if (chan->sweep_mode == MPS_SWEEP_MODE_UP)
	{
		if (chan->num < 6)
		{
			mps_fm_sweep(&chan->frequency, &chan->octave,
			             0xFFFF, 8, chan->sweep_speed);
		}
		else
		{
			mps_main_sweep_logic(&chan->frequency, 0,
			                     chan->sweep_speed);
		}
	}
	else if (chan->sweep_mode == MPS_SWEEP_MODE_DOWN)
	{
		if (chan->num < 6)
		{
			mps_fm_sweep(&chan->frequency, &chan->octave,
			             0, 0, chan->sweep_speed);
		}
		else
		{
			mps_main_sweep_logic(&chan->frequency, 0xFFFF,
			                     chan->sweep_speed);
		}
	} 
	else
	{
		chan->frequency = chan->target_frequency;
		chan->octave = chan->target_octave;
		chan->sweep_speed = 0;
	}
}

#endif
