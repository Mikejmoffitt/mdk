#include "rand.h"
#include "md/megadrive.h"

static uint16_t s_rand_value = 0x6502;

static inline void rand_step(void)
{
	const uint16_t feedback = (s_rand_value & 0x0001) ^
	                          ((s_rand_value >> 9) & 0x0001);
	s_rand_value = s_rand_value >> 1;
	s_rand_value |= (feedback << 15);
}

void srand(uint16_t seed)
{
	if (seed == 0) seed = vdp_get_hv_count();
	s_rand_value = seed;
}

uint16_t rand(void)
{
	rand_step();
	return s_rand_value;
}
