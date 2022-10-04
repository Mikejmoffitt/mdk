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

#include "md/megadrive.h"
#include "mdsdrv/mdsdrv.h"

#include <stdint.h>

#include "res.h"

// Sound driver work area.

#define MDS_WORK_SIZE 512
static uint16_t s_mds_work[MDS_WORK_SIZE];

uint16_t mds_init(const uint8_t *seqdata, const uint8_t *pcmdata)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register const uint8_t *a1 asm ("a1") = seqdata;
	register const uint8_t *a2 asm ("a2") = pcmdata;
	register uint16_t d0 asm ("d0") = 0;
	asm volatile (
		"jsr res_mdsdrv_bin+0"
		: "+a" (a0), "+a" (a1), "=r" (d0)
		: "a" (a2)
		: "d1", "cc");

	return d0;
}


void mds_request(MdsSlot slot, uint16_t id)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register uint16_t d0 asm ("d0") = id;
	register uint16_t d1 asm ("d1") = slot;
	asm volatile (
		"jsr res_mdsdrv_bin+8"
		:
		: "r" (d0), "r" (d1), "a" (a0)
		: "cc" );
}

uint32_t mds_command(uint16_t id, uint16_t param)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register uint32_t d0 asm ("d0") = id;
	register uint16_t d1 asm ("d1") = param;
	asm volatile (
		"jsr res_mdsdrv_bin+12"
		: "+r" (d0), "+r" (d1)
		: "a" (a0)
		: "a1", "d2", "cc");
	return d0;
}


uint32_t mds_command2(uint16_t id, uint16_t param1, uint16_t param2)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register uint32_t d0 asm ("d0") = id;
	register uint16_t d1 asm ("d1") = param1;
	register uint16_t d2 asm ("d2") = param2;
	asm volatile (
		"jsr res_mdsdrv_bin+12"
		: "+r" (d0), "+r" (d1), "+r" (d2)
		: "a" (a0)
		: "a1", "cc");
	return d0;
}

void mds_update(void)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register void *a6 asm ("a6"); // Hack since gcc didn't like clobbering the frame pointer
	asm volatile (
		"jsr res_mdsdrv_bin+4"
		: "=a" (a6)
		: "a" (a0)
		: "a1", "a2", "a3", "a4", "a5", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc");
}

const char *mds_get_version_str(void)
{
	register uint16_t *a0 asm ("a0") = s_mds_work;
	register uint32_t d0 asm ("d0") = MDS_CMD_GET_VERSION;
	asm volatile (
		"jsr res_mdsdrv_bin+12"
		: "+r" (d0), "+a" (a0)
		:
		: "a1", "d1", "d2", "cc");
	return (char*)a0;
}

void mds_pause(MdsSlot slot, uint8_t paused)
{
	mds_command2(MDS_CMD_SET_PAUSE, slot, paused);
}


void mds_fade(uint8_t target, uint8_t speed, uint8_t stop_when_done)
{
	mds_command(MDS_CMD_FADE_BGM, (speed << 8) | ((stop_when_done & 1) << 7) | (target & 0x7f));
}
