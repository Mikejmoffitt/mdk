#include "md/sysc_vctrl.h"
#include "md/mmio.h"

#include <stdint.h>

void md_sysc_vctrl_set(MdCVReg reg_data)
{
	volatile uint8_t *vctrl = (volatile uint8_t *)SYSC_PROTECTION_LOC_VCTRL;
	*vctrl = reg_data;
}
