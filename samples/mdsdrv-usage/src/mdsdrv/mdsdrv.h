//======================================================================
// MDSDRV API wrapper for MDK
//======================================================================
// Copyright (c) 2020 Ian Karlsson
// Source modified 2022 by Mike Moffitt
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must
//    not claim that you wrote the original software. If you use this
//    software in a product, an acknowledgment in the product
//    documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must
//    not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source
//    distribution.
//======================================================================

#ifndef MDSDRV_H
#define MDSDRV_H

#include "res.h"

// Music and sound effect priority slots.
typedef enum MdsSlot
{
	MDS_SLOT_BGM = 3,
	MDS_SLOT_SE1 = 2,
	MDS_SLOT_SE2 = 1,
	MDS_SLOT_SE3 = 0
} MdsSlot;

// Commands.
#define MDS_CMD_GET_CMD_CNT    0
#define MDS_CMD_GET_SOUND_CNT  1
#define MDS_CMD_GET_STATUS     2
#define MDS_CMD_GET_VERSION    3
#define MDS_CMD_GET_GTEMPO     4
#define MDS_CMD_SET_GTEMPO     5
#define MDS_CMD_GET_GVOLUME    6
#define MDS_CMD_SET_GVOLUME    7
#define MDS_CMD_WRITE_FM_PORT0 8
#define MDS_CMD_WRITE_FM_PORT1 9
#define MDS_CMD_FADE_BGM       10
#define MDS_CMD_SET_PAUSE      11
#define MDS_CMD_GET_VOLUME     12
#define MDS_CMD_SET_VOLUME     13
#define MDS_CMD_GET_TEMPO      14
#define MDS_CMD_SET_TEMPO      15

// Initializes the sound driver.
// This initializes the work area, unpacks and starts up the PCM driver. MDSDRV
// should then be updated every vblank using mds_update().
// seqdata must be word-aligned, while pcmdata must be 32k-aligned.
uint16_t mds_init(const uint8_t *seqdata, const uint8_t *pcmdata);

// Request playback of a sound.
// Specify 0 to stop the currently playing sound.
void mds_request(MdsSlot slot, uint16_t id);

// Command request for low-level access.
// Not all commands return a meaningful value, see MDSDRV
// documentation for details.
uint32_t mds_command(uint16_t id, uint16_t param);

// Command request for low-level access (command number >= 0x09).
// Not all commands return a meaningful value, see MDSDRV
// documentation for details.
uint32_t mds_command2(uint16_t id, uint16_t param1, uint16_t param2);

// MDSDRV update function. Must be called every vblank.
void mds_update(void);

// Returns a pointer to a NULL-terminated string containing version information.
const char *mds_get_version_str(void);

// Sets pause on/off.
void mds_pause(MdsSlot slot, uint8_t paused);

// Set BGM fade in/out.
// The volume of the BGM track will fade to the target level. The duration of
// the fade is as follows:
// (target - current) / (speed + 1)
// The target range is 0 to 127, with -0.75dB per step.
// 0 is the fastest fade speed, while 7 is the slowest.
void mds_fade(uint8_t target, uint8_t speed, uint8_t stop_when_done);

#endif
