/* mdk DMA control functions
Michael Moffitt 2018 */

#include "md/dma.h"
#include "md/macro.h"
#include "md/sys.h"
#include "md/vdp.h"

#define DMA_QUEUE_DEPTH 128
// Used with modulo operator, so should be power of 2.
_Static_assert(NUM_IS_POW2(DMA_QUEUE_DEPTH), "DMA queue depth != power of 2!");

typedef enum DmaOp DmaOp;
enum DmaOp
{
	DMA_OP_NONE,
	DMA_OP_TRANSFER,
	DMA_OP_SPR_TRANSFER,
	DMA_OP_COPY,
	DMA_OP_FILL
};

// Struct representing pre-calculated register values for the VDP's DMA.
typedef struct DmaCmd DmaCmd;
struct DmaCmd
{
	uint16_t stride;
	uint16_t len_1;
	uint16_t len_2;
	uint16_t src_1;  // Used as data for DMA fill.
	uint16_t src_2;
	uint16_t src_3;
	uint32_t ctrl;
} __attribute__ ((aligned (0x10)));

// DMA queue ring buffer.
static uint8_t s_dma_q_write_idx;
static uint8_t s_dma_q_read_idx;
static DmaCmd s_dma_q[DMA_QUEUE_DEPTH];

// Special high priority sprite list(s) queue.
static uint8_t s_dma_prio_q_idx;
static DmaCmd s_dma_prio_q_cmd[8];

void md_dma_init(void)
{
	s_dma_q_read_idx = 0;
	s_dma_q_write_idx = 0;
	s_dma_prio_q_idx = 0;
}

// Calculate required register values for a transfer
static inline void enqueue_int(DmaOp op, uint32_t bus, uint16_t dest,
                               uint32_t src, uint16_t n, uint16_t stride)
{
	// A command slot is chosen from one of the two queues, based on the type.
	DmaCmd *cmd;
	if (op == DMA_OP_SPR_TRANSFER)
	{
		if (s_dma_prio_q_idx >= ARRAYSIZE(s_dma_prio_q_cmd)) return;
		cmd = &s_dma_prio_q_cmd[s_dma_prio_q_idx];
		s_dma_prio_q_idx++;
	}
	else
	{
		cmd = &s_dma_q[s_dma_q_write_idx];
		s_dma_q_write_idx = (s_dma_q_write_idx + 1) %
		                     ARRAYSIZE(s_dma_q);
		if (s_dma_q_write_idx == s_dma_q_read_idx) return;
	}

	// DMA register values are calculated ahead of time to be consumed during
	// VBlank faster.
	cmd->stride = VDP_REGST(VDP_AUTOINC, stride);
	cmd->len_1 = VDP_REGST(VDP_DMALEN1, n & 0xFF);
	cmd->len_2 = VDP_REGST(VDP_DMALEN2, (n >> 8) & 0xFF);

	switch (op)
	{
		case DMA_OP_NONE:
			return;

		case DMA_OP_TRANSFER:
		case DMA_OP_SPR_TRANSFER:
			src = src >> 1;
			cmd->src_1 = VDP_REGST(VDP_DMASRC1, src & 0xFF);
			src = src >> 8;
			cmd->src_2 = VDP_REGST(VDP_DMASRC2, src & 0xFF);
			src = src >> 8;
			cmd->src_3 = VDP_REGST(VDP_DMASRC3, src & 0x7F);
			break;

		case DMA_OP_FILL:
			cmd->src_1 = src & 0xFFFF;  // desired data.
			cmd->src_3 = VDP_REGST(VDP_DMASRC3, VDP_DMA_SRC_FILL);
			break;

		case DMA_OP_COPY:
			cmd->src_1 = VDP_REGST(VDP_DMASRC1, src & 0xFF);
			cmd->src_2 = VDP_REGST(VDP_DMASRC2, (src >> 8) & 0xFF);
			cmd->src_3 = VDP_REGST(VDP_DMASRC3, VDP_DMA_SRC_COPY);
			break;
	}

	cmd->ctrl = VDP_CTRL_DMA_BIT | VDP_CTRL_ADDR(dest) | bus;
}

static inline void md_dma_enqueue(DmaOp op, uint32_t bus, uint16_t dest,
                                  uint32_t src, uint16_t n, uint16_t stride)
{
	if (op != DMA_OP_TRANSFER && op != DMA_OP_SPR_TRANSFER)
	{
		enqueue_int(op, bus, dest, src, n, stride);
		return;
	}
	// check that the source address + length won't cross a 128KIB boundary
	// based on SGDK's DMA validation.
	const uint32_t limit = 0x20000 - (src & 0x1FFFF);

	// If the transfer will cross the 128KiB boundary, transfer the latter
	// half first, then truncate the transfer's length to fill the rest.
	if (n > (limit >> 1))
	{
		enqueue_int(op, bus,
		            dest + limit,
		            src + limit,
		            n - (limit >> 1), stride);
		n = limit >> 1;
	}
	enqueue_int(op, bus, dest, src, n, stride);
}


// Schedule a DMA for next vblank from 68K mem to VRAM
void md_dma_transfer_vram(uint16_t dest, const void *src, uint16_t words,
                          uint16_t stride)
{
	md_dma_enqueue(DMA_OP_TRANSFER, VDP_CTRL_VRAM_WRITE,
	              dest, (uint32_t)src, words, stride);
}

void md_dma_transfer_cram(uint16_t dest, const void *src, uint16_t words,
                          uint16_t stride)
{
	md_dma_enqueue(DMA_OP_TRANSFER, VDP_CTRL_CRAM_WRITE,
	              dest, (uint32_t)src, words, stride);
}

void md_dma_transfer_vsram(uint16_t dest, const void *src, uint16_t words,
                           uint16_t stride)
{
	md_dma_enqueue(DMA_OP_TRANSFER, VDP_CTRL_VSRAM_WRITE,
	              dest, (uint32_t)src, words, stride);
}

void md_dma_transfer_spr_vram(uint16_t dest, const void *src, uint16_t words,
                              uint16_t stride)
{
	md_dma_enqueue(DMA_OP_SPR_TRANSFER, VDP_CTRL_VRAM_WRITE,
	              dest, (uint32_t)src, words, stride);
}

// Schedule a DMA for next vblank to fill specified bytes at dest with val.
void md_dma_fill_vram(uint16_t dest, uint16_t val, uint16_t bytes, uint16_t stride)
{
	md_dma_enqueue(DMA_OP_FILL, VDP_CTRL_VRAM_WRITE, dest, val, bytes, stride);
}

// Schedule a DMA for next vblank to copy specified bytes from VRAM src to VRAM dest.
void md_dma_copy_vram(uint16_t dest, uint16_t src, uint16_t bytes, uint16_t stride)
{
	md_dma_enqueue(DMA_OP_COPY, VDP_CTRL_VRAM_WRITE, dest, src, bytes, stride);
}

void md_dma_process_cmd(DmaCmd *cmd);  // dma_process.a68

#ifdef MDK_FAKE_DMA_TEST_HACK
static void dma_fake(DmaCmd *cmd)
{
//	if (cmd->op != DMA_OP_TRANSFER && cmd->op != DMA_OP_SPR_TRANSFER)
//	{
//		md_dma_process_cmd(cmd);
//		return;
//	}
	// TODO: support software fill
	const uint32_t src_addr = (cmd->src_3 << 17) | (cmd->src_2 << 9) | (cmd->src_1 << 1);
	const uint16_t *data = (const uint16_t *)src_addr;
	const uint32_t ctrl = cmd->ctrl & ~(VDP_CTRL_DMA_BIT);
	md_vdp_set_autoinc(cmd->stride);
	VDPPORT_CTRL32 = ctrl;
	const uint16_t len = (cmd->len_2 << 8) | cmd->len_1;
	for (uint16_t i = 0; i < len; i++)
	{
		md_vdp_write(*data++);
	}
}
#endif

void md_dma_process(void)
{
	md_vdp_wait_dma();
	MD_SYS_BARRIER();

	const bool ints_enabled = md_sys_di();

	// Process high-priority slots first.
	for (uint16_t i = 0; i < ARRAYSIZE(s_dma_prio_q_cmd); i++)
	{
		if (s_dma_prio_q_cmd[i].stride == 0) break;
#ifdef MDK_FAKE_DMA_TEST_HACK
		dma_fake(&s_dma_prio_q_cmd[i]);
#else
		md_dma_process_cmd(&s_dma_prio_q_cmd[i]);
#endif
	}
	s_dma_prio_q_idx = 0;

	// Process all queued transfers.
	while (s_dma_q_read_idx != s_dma_q_write_idx)
	{
		DmaCmd *cmd = &s_dma_q[s_dma_q_read_idx];
		s_dma_q_read_idx = (s_dma_q_read_idx + 1) % DMA_QUEUE_DEPTH;
#ifdef MDK_FAKE_DMA_TEST_HACK
		dma_fake(cmd);
#else
		md_dma_process_cmd(cmd);
#endif
	}

	if (ints_enabled) md_sys_ei();
}
