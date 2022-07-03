/* md-toolchain DMA control functions
Michael Moffitt 2018-2021

For large lengths of data, like character graphics, mappings, scroll tables,
and palettes, a DMA is the best way to manipulate the VDP's VRAM. DMA
operations are fastest during the vertical vblanking interval (vblank), or
when the display has been disabled (see vdp_set_display_en).

A DMA operation may be scheduled at any time during active display, and placed
in a queue of pending operations, using the following functions:

    dma_q_transfer_vram     dma_q_transfer_cram    dma_q_transfer_vsram
    dma_q_fill_vram
    dma_q_copy_vram

At the start of every vblank, dma_q_process() should be called, which will
execute all scheduled DMA operations. DMA transfer operations hog the main CPU
bus, while fill and copy operations do not. Try to ensure that your queued VRAM
transfer operations complete within the blanking period.

*/
#ifndef MD_DMA_H
#define MD_DMA_H

#include "md/vdp.h"

void dma_q_init(void);

// Schedule a DMA for next vblank from 68K mem to VRAM
void dma_q_transfer_vram(uint16_t dest, const void *src, uint16_t words,
                         uint16_t stride);
void dma_q_transfer_cram(uint16_t dest, const void *src, uint16_t words,
                         uint16_t stride);
void dma_q_transfer_vsram(uint16_t dest, const void *src, uint16_t words,
                          uint16_t stride);
// Special high-priority slot for sprite table. Schedules a transfer that will
// run before any others.
void dma_q_transfer_spr_vram(uint16_t dest, const void *src, uint16_t words,
                             uint16_t stride);

// Schedule a DMA for next vblank to fill n words at dest with val.
void dma_q_fill_vram(uint16_t dest, uint16_t val, uint16_t bytes, uint16_t stride);

// Schedule a DMA for next vblank to copy n words from VRAM src to VRAM dest.
void dma_q_copy_vram(uint16_t dest, uint16_t src, uint16_t bytes, uint16_t stride);

// Internal Use ---------------------------------------------------------------

// Process any queued DMA requests.
void dma_q_process(void);

#endif // MD_DMA_H
