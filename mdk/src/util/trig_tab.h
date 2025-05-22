#pragma once
#include "util/fixed.h"
#define TRIG_TAB_ATAN_INPUT_RANGE 64

// Range 0 to 2 * pi
extern const fix16_t trig_tab_sin[256];
extern const fix16_t trig_tab_cos[256];
extern const fix16_t trig_tab_tan[256];
// Range 0 to 64 (one quadrant)
extern const fix16_t trig_tab_atan[4096];
