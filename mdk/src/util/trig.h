#pragma once

// Fixed point trigonometry.

#include "util/fixed.h"
#include "trig_tab.h"

#define TRIG_FIX16_PI INTTOFIX16((3.1415926535897))

#define DEGTOUINT8(deg) ((int)(((deg) / 360.0) * 256) % 256)
#define RADTOUINT8(rad) ((int)(((rad) / 2 * 3.14159) * 256) % 256)

// sin, cos, and tan accept an angle from 0 - 255, representing 0 - 2 * pi.
static inline fix16_t trig_sin(uint8_t angle)
{
	return trig_tab_sin[angle];
}

static inline fix16_t trig_cos(uint8_t angle)
{
	return trig_tab_cos[angle];
}

static inline fix16_t trig_tan(uint8_t angle)
{
	return trig_tab_tan[angle];
}

uint8_t trig_atan(int y, int x);
