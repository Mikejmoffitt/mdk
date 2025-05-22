#pragma once
#define MDK_NUM_IS_POW2(x) ((x & (x - 1)) == 0)

#define MDK_ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

#define MDK_BITVAL(x) (1 << x)
#define MDK_BTST(x, n) ((x) & (MDK_BITVAL(n)))
#define MDK_BCLR(x, n) ((x) &= (~(MDK_BITVAL(n))))
#define MDK_BSET(x, n) ((x) |= (MDK_BITVAL(n)))

#define MDK_MAX(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

#define MDK_MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })
