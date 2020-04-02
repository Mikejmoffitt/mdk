// md-toolchain fixed point functions
// Michael Moffitt 2018
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

// Fixed point types
typedef int32_t fix32_t;
typedef int16_t fix16_t;

// Fixed point conversions
#define INTTOFIX32(x) ((fix32_t)((x) * FIX32_MUL))
#define FIX32TOINT(x) ((int)((x) / FIX32_MUL))
#define INTTOFIX16(x) ((fix16_t)((x) * FIX16_MUL))
#define FIX16TOINT(x) ((int)((x) / FIX16_MUL))

// Precision configuration
#define FIX32_PRECISION_BITS 8
#define FIX32_MUL (1 << FIX32_PRECISION_BITS)

#define FIX16_PRECISION_BITS 8
#define FIX16_MUL (1 << FIX16_PRECISION_BITS)

#endif // FIXED_H
