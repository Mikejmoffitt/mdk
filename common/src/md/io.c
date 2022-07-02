/* md-toolchain I/O peripheral support
Michael Moffitt 2018 */
#include "md/io.h"
#include "md/mmio.h"
#include "md/sys.h"

uint16_t g_pad_cache[2];

uint16_t md_io_pad_read(uint8_t port)
{
	return g_pad_cache[port];
}
