/* mdk SRAM functions
Michael Moffitt 2018-2023

SRAM access functions. These functions exist in low ROM and handle SRAM
enable/disable automatically, so the caller does not need to be conscious
of code location.

The intended use case is to allow the user to copy to and from a struct
representing save data.

*/

#ifndef MD_SRAM_H
#define MD_SRAM_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

// Copies n bytes from 68000 memory space src_addr to dest_offset within SRAM.
void md_sram_write(uint32_t dest_offset, const void *src_addr, uint32_t n);

// Copies n bytes from src_offset in SRAM to dest_addr in 68000 memory space.
void md_sram_read(uint32_t src_offset, void *dest_addr, uint32_t n);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MD_SRAM_H
