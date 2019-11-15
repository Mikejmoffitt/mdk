/* md-toolchain DMA control functions
Michael Moffitt 2018

For large lengths of data, like character graphics, mappings, scroll tables,
and palettes, a DMA is the best way to manipulate the VDO's VRAM. DMA
operations are fastest during the vertical vblanking interval (vblank), or
when the display has been disabled (see vdp_set_display_en).

There are two ways to use these DMA functions.

1) Scheduled DMA (recommended)

A DMA operation may be scheduled at any time during active display, and placed
in a queue of pending operations, using the following functions:

    dma_q_transfer_vram     dma_q_transfer_cram    dma_q_transfer_vsram
    dma_q_fill_vram
    dma_q_copy_vram

At the start of every vblank, dma_q_process()
should be called, which will run as many DMA operations as possible during
vblank. dma_q_process() attempts to only transfer up to the maximum bandwidth
possible during vertical blank, and any transfers that could not be completed
in that amount of time will get pushed to the next vertical blank.

DMA transfer operations hog the main CPU bus, while fill and copy operations do
not. A good strategy might be to run copy and fill operations without the
scheduler, as it exists mostly to benefit transfer operations.

2) Immediate DMA

A DMA operation will be run immediately upon calling the following functions:

    dma_transfer_vram       dma_transfer_cram      dma_transfer_vsram
    dma_fill_vram
    dma_copy_vram

The only differences between these and the queued variants is that they are run
immediately, and do not take a stride argument. It is recommended that the
display is disabled, or that they are only run during vblank, so as to avoid
visible artifacts (especially for CRAM) and to better utilize bandwidth.

Stride should be set before calling a DMA with dma_set_stride. This argument is
omitted from the function call for performance reasons, as the primary use case
for these functions is for transferring large amounts of statically defined
data, during an initialization (for example).

It may be wise to use the immdiate fill and copy operations in conjunction with
scheduled DMA transfers in a real-world scenario.

*/
#ifndef DMA_H
#define DMA_H

#include "md/vdp.h"

// Values for bus type
#define DMA_OP_BUS_CRAM 0
#define DMA_OP_BUS_VRAM 1
#define DMA_OP_BUS_VSRAM 2

#define DMA_Q_BUDGET_AUTO 0x00000000

// Block on DMA completion.
static inline void dma_wait(void);

// Configure the DMA queue vblank transfer budget for max_words per vblank.
// DMA_Q_BUDGET_AUTO uses a calculation based on the current mode.
void dma_q_set_budget(uint32_t max_words);

// Schedule a DMA for next vblank from 68K mem to VRAM
void dma_q_transfer_vram(uint16_t dest, void *src, uint16_t n, uint16_t stride);
void dma_q_transfer_cram(uint16_t dest, void *src, uint16_t n, uint16_t stride);
void dma_q_transfer_vsram(uint16_t dest, void *src, uint16_t n, uint16_t stride);

// Schedule a DMA for next vblank to fill n words at dest with val.
void dma_q_fill_vram(uint16_t dest, uint16_t val, uint16_t n, uint16_t stride);

// Schedule a DMA for next vblank to copy n words from VRAM src to VRAM dest.
void dma_q_copy_vram(uint16_t dest, uint16_t src, uint16_t n, uint16_t stride);

// Run at the start of Vblank to process pending DMA requests.
void dma_q_process(void);

// Remove any remaining pending DMA transfers from the DMA queue.
void dma_q_flush(void);

// Set the post-inc address value.
static inline void dma_set_stride(uint16_t stride);

// Run a DMA copy. n is in words.
void dma_transfer(uint16_t bus, uint16_t dest, void *src, uint16_t n);

// Run a DMA fill. n is in words.
void dma_fill(uint16_t bus, uint16_t dest, uint16_t val, uint16_t n);

// Run a VRAM-to-VRAM DMA copy. n is in words.
void dma_copy(uint16_t bus, uint16_t dest, uint16_t src, uint16_t n);

// Copy n words from 68K mem src to VDP vram_dest immediately.
static inline void dma_transfer_vram(uint16_t dest, void *src, uint16_t n);
static inline void dma_transfer_cram(uint16_t dest, void *src, uint16_t n);
static inline void dma_transfer_vsram(uint16_t dest, void *src, uint16_t n);

// Fill n bytes at dest with val immediately.
static inline void dma_fill_vram(uint16_t dest, uint16_t val, uint16_t n);

// Copy n words from VDP VRAM src to VDP VRAM dest immediately.
static inline void dma_copy_vram(uint16_t dest, uint16_t src, uint16_t n);

// ----------------------------------------------------------------------------
static inline void dma_wait(void)
{
	vdp_wait_dma();
}

static inline void dma_set_stride(uint16_t stride)
{
	vdp_set_autoinc(stride);
}

static inline void dma_transfer_cram(uint16_t dest, void *src, uint16_t n)
{
	dma_transfer(DMA_OP_BUS_CRAM, dest, src, n);
}

static inline void dma_transfer_vram(uint16_t dest, void *src, uint16_t n)
{
	dma_transfer(DMA_OP_BUS_VRAM, dest, src, n);
}

static inline void dma_transfer_vsram(uint16_t dest, void *src, uint16_t n)
{
	dma_transfer(DMA_OP_BUS_VSRAM, dest, src, n);
}

static inline void dma_fill_vram(uint16_t dest, uint16_t val, uint16_t n)
{
	dma_fill(DMA_OP_BUS_VRAM, dest, val, n);
}

static inline void dma_copy_vram(uint16_t dest, uint16_t src, uint16_t n)
{
	dma_copy(DMA_OP_BUS_VRAM, dest, src, n);
}
#endif // DMA_H
