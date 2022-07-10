/* mdk uPD7759 support (for System C2)
Michael Moffitt 2022 */
#ifndef MD_UPD7759_H
#define MD_UPD7759_H

#include <stdint.h>

// Despite the naming convention of md_*, these functions are only useful for
// System C2, where a uPD7759 is mapped into the 68000 address space.

// Reset the uPD7759. This must be done after a bank switch for it to
// properly register the change in the voice table (if applicable).
// Reset line is asserted (brought to 0) when reset = 1.
// This function wraps md_ioc_set_upd7759_reset().
void md_adpcm_set_reset(uint16_t reset);

// Set a phrase request value for the uPD7759.
void md_adpcm_play(uint8_t value);

// Select bank 0-3. Should call md_adpcm_reset() afterwards, unless the banks
// don't change the voice table contents.
// This function wraps md_ioc_set_udp7759_bank().
void md_adpcm_set_bank(uint16_t bank);

// Returns 1 if the uPD7759 IC is busy. Wraps md_ioc_get_upd7759_busy().
uint16_t md_adpcm_busy(void);

#endif
