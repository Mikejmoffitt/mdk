#ifndef UTIL_RAND_H
#define UTIL_RAND_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#include <stdint.h>

void srand(uint16_t seed);
uint16_t rand(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // RAND_H
