/* mdk SRAM functions
Michael Moffitt 2018-2022

SRAM access functions. These functions exist in low ROM and handle SRAM
enable/disable automatically, so the caller does not need to be conscious
of code location.

The intended use case is to allow the user to copy to and from a struct
representing save data.

*/

#ifndef MD_SRAM_H
#define MD_SRAM_H

// Copies n bytes from 68000 memory space src_addr to dest_offset within SRAM.
void md_sram_write(uint32_t dest_offset, const void *src_addr, uint32_t n);

// Copies n bytes from src_offset in SRAM to dest_addr in 68000 memory space.
void md_sram_read(uint32_t src_offset, void *dest_addr, uint32_t n);

// Set n bytes, starting from offset in SRAM, to constant value c.
void md_sram_set(uint32_t dest_offset, uint16_t c, uint32_t n);

#endif  // MD_SRAM_H
