#include "rand.h"
#include "md/megadrive.h"

uint16_t g_rand_value = 0x6502;

void srand(uint16_t seed)
{
	if (seed == 0) seed = md_vdp_get_hv_count();
	g_rand_value = seed;
}
