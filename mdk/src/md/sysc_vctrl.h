#pragma once

#ifdef MDK_TARGET_C2

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdbool.h>
#include "md/mmio.h"


extern uint8_t g_c2_vctrl;

// When blank is true, the output from the video DAC is blanked.
static inline void md_sysc_vctrl_set_blank(bool blank);

// Drives the protection reset line low.
static inline void md_sysc_vctrl_set_protection_reset(bool reset);

// When compat is true, the palette is arranged in a way that is similar to the
// Megadrive's color RAM, with one nybble per color channel, plus an upper
// nybble to provide the LSB of color data.
// When compat is false, RGB data is directly packed in order in a 16-bit word.
static inline void md_sysc_vctrl_set_md_color_compat(bool compat);

// -----------------------------------------------------------------------------

static inline void md_sysc_vctrl_write(uint8_t data)
{
	volatile uint8_t *vctrl = (volatile uint8_t *)SYSC_PROTECTION_LOC_VCTRL;
	*vctrl = data;
}

static inline void md_sysc_vctrl_set_blank(bool blank)
{
	g_c2_vctrl &= ~0x01;
	g_c2_vctrl |= blank ? 0x01 : 0x00;
	md_sysc_vctrl_write(g_c2_vctrl);
}

static inline void md_sysc_vctrl_set_protection_reset(bool reset)
{
	g_c2_vctrl &= ~0x02;
	g_c2_vctrl |= reset ? 0x02 : 0x00;
	md_sysc_vctrl_write(g_c2_vctrl);
}

static inline void md_sysc_vctrl_set_md_color_compat(bool compat)
{
	g_c2_vctrl &= ~0x04;
	g_c2_vctrl |= compat ? 0x04 : 0x00;
	md_sysc_vctrl_write(g_c2_vctrl);
}

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MDK_TARGET_C2
