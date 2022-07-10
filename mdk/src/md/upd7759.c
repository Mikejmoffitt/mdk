#include "md/upd7759.h"
#include "md/mmio.h"
#include "md/ioc.h"

// Despite the naming convention of md_*, these functions are only useful for
// System C2, where a uPD7759 is mapped into the 68000 address space.


// Reset the uPD7759. This must be done after a bank switch for it to
// properly register the change in the voice table (if applicable).
// Reset line is asserted (brought to 0) when reset = 1.
// This function wraps md_ioc_set_upd7759_reset().
void md_adpcm_set_reset(uint16_t reset)
{
	md_ioc_set_upd7759_reset(reset);
}

// Set a phrase request value for the uPD7759.
void md_adpcm_select(uint8_t value)
{
	volatile uint8_t *phrase = (volatile uint8_t *)SYSC_IO_UPD7759_PHRASE_LOC;
	*phrase = value;
}

// Select bank 0-3. Should call md_adpcm_reset() afterwards, unless the banks
// don't change the voice table contents.
// This function wraps md_ioc_set_udp7759_bank().
void md_adpcm_set_bank(uint16_t bank)
{
	md_ioc_set_udp7759_bank(bank);
}

// Returns 1 if the uPD7759 IC is busy.
uint16_t md_adpcm_busy(void)
{
	return md_ioc_get_upd7759_busy();
}
