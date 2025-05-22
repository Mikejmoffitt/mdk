#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>

extern uint16_t g_rand_value;

void srand(uint16_t seed);
static inline uint16_t rand(void);

// -----------------------------------------------------------------------------

static inline void rand_step(void)
{
	const uint16_t feedback = (g_rand_value & 0x0001) ^
	                          ((g_rand_value >> 9) & 0x0001);
	g_rand_value = g_rand_value >> 1;
	g_rand_value |= (feedback << 15);
}

static inline uint16_t rand(void)
{
	rand_step();
	return g_rand_value;
}

#ifdef __cplusplus
}
#endif  // __cplusplus
