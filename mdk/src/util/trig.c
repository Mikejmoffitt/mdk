#include "trig.h"
#include "util/fixed.h"
#include "md/megadrive.h"

static uint8_t atan_int(fix32_t ratio)
{
	if (ratio >= INTTOFIX16(TRIG_TAB_ATAN_INPUT_RANGE))
	{
		return trig_tab_atan[ARRAYSIZE(trig_tab_atan) - 1];
	}
	if (ratio < 0) return 0;
	ratio = ratio / 4;
	return trig_tab_atan[ratio];
}

uint8_t trig_atan(int y, int x)
{
	y = -y;
	if (x == 0)
	{
		return y < 0 ? DEGTOUINT8(270) : DEGTOUINT8(90);
	}
	else
	{
		if (x >= 0)
		{
			if (y >= 0)
			{
				const fix32_t ratio = FIX32DIV(y, x);
				return atan_int(ratio);
			}
			else
			{
				const fix32_t ratio = FIX32DIV(-y, x);
				const uint8_t angle = atan_int(ratio);
				return DEGTOUINT8(270) + (DEGTOUINT8(90) - angle);
			}
		}
		else
		{
			if (y >= 0)
			{
				const fix32_t ratio = FIX32DIV(y, -x);
				const uint8_t angle = atan_int(ratio);
				return DEGTOUINT8(180) - angle;
			}
			else
			{
				const fix32_t ratio = FIX32DIV(-y, -x);
				const uint8_t angle = atan_int(ratio);
				return DEGTOUINT8(180) + angle;
			}
		}
	}
}
