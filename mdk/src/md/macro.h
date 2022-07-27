#ifndef MD_MACRO_H
#define MD_MACRO_H

#define NUM_IS_POW2(x) ((x & (x - 1)) == 0)

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif  // ARRAYSIZE

#ifndef MAX
#define MAX(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })
#endif  // MAX

#ifndef MIN
#define MIN(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })
#endif  // MIN

#endif  // MD_MARCO_H
