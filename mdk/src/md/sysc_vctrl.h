#ifndef MD_SYSC_VCTRL
#define MD_SYSC_VCTRL

#include <stdint.h>

void md_sysc_vctrl_set_blank(uint16_t blank);
void md_sysc_vctrl_set_protection_reset(uint16_t reset);
void md_sysc_vctrl_set_md_color_compat(uint16_t compat);

#endif  // MD_SYSC_VCTRL
