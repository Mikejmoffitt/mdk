#ifndef MD_SYSC_VCTRL
#define MD_SYSC_VCTRL

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

void md_sysc_vctrl_set_blank(uint16_t blank);
void md_sysc_vctrl_set_protection_reset(uint16_t reset);
void md_sysc_vctrl_set_md_color_compat(uint16_t compat);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MD_SYSC_VCTRL
