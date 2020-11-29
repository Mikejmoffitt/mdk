// md-toolchain fixed point functions
// Based loosely on the fixed-point helper macros from SGDK.
// Michael Moffitt 2018
#ifndef FIXED_H
#define FIXED_H

#include <stdint.h>

// Fixed point types
typedef int32_t fix32_t;
typedef int16_t fix16_t;

// Precision configuration
#define FIX32_PRECISION_BITS 8
#define FIX32_COEF (1 << FIX32_PRECISION_BITS)

#define FIX16_PRECISION_BITS 8
#define FIX16_COEF (1 << FIX16_PRECISION_BITS)

// Fixed point conversions
#define INTTOFIX32(x) ((fix32_t)((x) * FIX32_COEF))
#define FIX32TOINT(x) ((int)((x) / FIX32_COEF))
#define INTTOFIX16(x) ((fix16_t)((x) * FIX16_COEF))
#define FIX16TOINT(x) ((int)((x) / FIX16_COEF))

// Fixed point multiplication and division
// TODO: We need libGCC or something in here for the multiplication
#define FIX16MUL(x, y) (((x) * (y)) >> FIX16_PRECISION_BITS)
#define FIX16DIV(x, y) (((x) << FI16_PRECISION_BITS) / (y))
#define FIX32MUL(x, y) (((x) * (y)) >> FIX32_PRECISION_BITS)
#define FIX32DIV(x, y) (((x) << FI32_PRECISION_BITS) / (y))

#endif // FIXED_H
