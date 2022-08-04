/* mdk I/O peripheral support
Michael Moffitt 2018 */
#include "md/io.h"
#include "md/mmio.h"
#include "md/sys.h"

// Even bytes are state, odd bytes are previous frame's state.
uint16_t g_md_pad[2];
uint16_t g_md_pad_prev[2];
uint16_t g_md_pad_pos[2];
uint16_t g_md_pad_neg[2];
