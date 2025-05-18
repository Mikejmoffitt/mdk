#pragma once

#include <stdint.h>

#ifdef MDK_BOOT_DEBUG
#define BEEPCHECK(x) \
	md_test_beep_c(0xF | (x << 4), 8-1)
#else
#define BEEPCHECK(x) (void)x
#endif  // BEEPCHECK
