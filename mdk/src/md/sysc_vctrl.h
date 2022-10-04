#ifndef MD_SYSC_VCTRL
#define MD_SYSC_VCTRL

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdbool.h>

// When blank is true, the output from the video DAC is blanked.
void md_sysc_vctrl_set_blank(bool blank);

// Drives the protection reset line low.
void md_sysc_vctrl_set_protection_reset(bool reset);

// When compat is true, the palette is arranged in a way that is similar to the
// Megadrive's color RAM, with one nybble per color channel, plus an upper
// nybble to provide the LSB of color data.
// When compat is false, RGB data is directly packed in order in a 16-bit word.
void md_sysc_vctrl_set_md_color_compat(bool compat);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MD_SYSC_VCTRL
