#ifndef MPS_PLAY_H
#define MPS_PLAY_H

#define MPS_CHANCNT 10

#define MPS_STATUS_IDLE 0
#define MPS_STATUS_PLAY 1

#define MPS_PAN_LEFT 0x80
#define MPS_PAN_RIGHT 0x40
#define MPS_PAN_BOTH (MPS_PAN_LEFT | MPS_PAN_RIGHT);

#define MPS_FX_SWEEP_UP 0x01
#define MPS_FX_SWEEP_DOWN 0x02
#define MPS_FX_SWEEP_PORTA 0x03
#define MPS_FX_VIBRATO 0x04
#define MPS_FX_PAN 0x08
#define MPS_FX_JUMP 0x0B
#define MPS_FX_DAC 0x17
#define MPS_FX_NOISE 0x20

#define MPS_SWEEP_MODE_OFF 0
#define MPS_SWEEP_MODE_UP 1
#define MPS_SWEEP_MODE_DOWN 2
#define MPS_SWEEP_MODE_PORTA 3

#define MPS_PORTA_NONE 0xFF

#define VIB_SPEED_MOD 4
#define VIB_PSG_DIV 4
#define VIB_PHASE_UP 0
#define VIB_PHASE_DOWN 1

#define PSG_STATE_OFF 0
#define PSG_STATE_ON 1
#define PSG_LOOP_NONE 0xFF

#define KEY_OFF 0
#define KEY_ON 1

#include "mps/mps_types.h"

typedef struct MpsChannelState
{
	// Channel column number
	uint8_t num;
	// Track progress variables
	uint16_t *arrangement; // Points to the top of pattern list (offset from track)
	uint8_t pattern_idx; // Position in arrangement, from top to bottom

	MpsCell *current_pattern; // Position in pattern pointed to by arrangement
	uint8_t row_idx; // Position in pattern; when >= pattern_length, go to next

	uint16_t tick_delay; // Set to (time_base * tick) when playing a note.
	uint8_t nops_count;

	uint8_t prev_instrument_id;
	MpsInstrument *instrument;

//	uint8_t note;
	// To be run during frequency commits
	uint8_t key_state;

	uint8_t volume; // 0 - 7F, or 0 - F, depending on FM / PSG

	// PSG envelopes
	uint8_t psg_env_index;
	uint8_t psg_env_loop;

	// Frequency management
	uint8_t sweep_mode;
	uint8_t sweep_speed;
	uint16_t frequency; // Derived from note+octave; is period for PSG.
	uint16_t target_frequency;
	uint8_t octave;
	uint16_t target_octave;

	// Triangle Vibrato
	uint8_t vib_nop; // Passes before running counters
	uint8_t vib_depth; // rate of change in triangle wave
	uint8_t vib_speed; // Phase counter max length

	uint8_t vib_phase; // Phase counter.
	int8_t vib_offset; // Frequency offset to be applied

	// TODO: Handle effects
	uint8_t panning;
} MpsChannelState;

typedef struct MpsState
{
	uint8_t status;
	MpsHeader *track;
	MpsInstrument *instruments;
	MpsChannelState channels[MPS_CHANCNT];
	uint8_t dac_en;
} MpsState;

// Set up music data for playback
void mps_init(const MpsHeader *track);

// Start playing music
void mps_play(void);

// Stop playing music
void mps_stop(void);

// Run one frame of music playback. Usually done during vblank.
void mps_tick(void);

// Get metadata about the track.
MpsHeader *mps_get_header(void);

// Get channel state
MpsChannelState *mps_get_chanstate(uint8_t channel);

#endif
