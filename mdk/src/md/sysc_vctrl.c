#include "md/sysc_vctrl.h"
#include "md/mmio.h"

#include <stdint.h>

static uint8_t s_vctrl_cache = 0x07;

static inline void md_sysc_vctrl_write(uint8_t data)
{
	volatile uint8_t *vctrl = (volatile uint8_t *)SYSC_PROTECTION_LOC_VCTRL;
	*vctrl = data;
}

void md_sysc_vctrl_set_blank(bool blank)
{
	s_vctrl_cache &= ~0x01;
	s_vctrl_cache |= blank ? 0x01 : 0x00;
	md_sysc_vctrl_write(s_vctrl_cache);
}

void md_sysc_vctrl_set_protection_reset(bool reset)
{
	s_vctrl_cache &= ~0x02;
	s_vctrl_cache |= reset ? 0x02 : 0x00;
	md_sysc_vctrl_write(s_vctrl_cache);
}

void md_sysc_vctrl_set_md_color_compat(bool compat)
{
	s_vctrl_cache &= ~0x04;
	s_vctrl_cache |= compat ? 0x04 : 0x00;
	md_sysc_vctrl_write(s_vctrl_cache);
}
