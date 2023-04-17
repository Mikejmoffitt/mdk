// mdk fixed point
// Michael Moffitt 2018-2022
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

// Fixed point types
typedef int32_t fix32_t;
typedef int16_t fix16_t;

#ifndef MD_FIXED_BITS
#define MD_FIXED_BITS 4
#endif  // MD_FIXED_BITS

// Precision configuration
#define FIX32_PRECISION_BITS MD_FIXED_BITS
#define FIX32_COEF (1 << FIX32_PRECISION_BITS)

#define FIX16_PRECISION_BITS MD_FIXED_BITS
#define FIX16_COEF (1 << FIX16_PRECISION_BITS)

// TODO: Unify these into storage-ambiguous macros.

// Fixed point conversions
#define INTTOFIX32(x) ((fix32_t)((x) * FIX32_COEF))
#define FIX32TOINT(x) ((int)((x) / FIX32_COEF))
#define INTTOFIX16(x) ((fix16_t)((x) * FIX16_COEF))
#define FIX16TOINT(x) ((int)((x) / FIX16_COEF))

// Fixed point multiplication and division
#define FIX16MUL(x, y) (((x) * (y)) >> FIX16_PRECISION_BITS)
#define FIX16DIV(x, y) (((x) << FIX16_PRECISION_BITS) / (y))
#define FIX32MUL(x, y) (((x) * (y)) >> FIX32_PRECISION_BITS)
#define FIX32DIV(x, y) (((x) << FIX32_PRECISION_BITS) / (y))

#endif // FIXED_H
