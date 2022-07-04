/* md-toolchain DMA control functions
Michael Moffitt 2018 */

#include "md/dma.h"
#include "md/vdp.h"
#include "md/sys.h"

// This should be a power of 2, since it is used with the modulo operator.
#define DMA_QUEUE_DEPTH 256

typedef enum DmaOp
{
	DMA_OP_NONE,
	DMA_OP_TRANSFER,
	DMA_OP_FILL,
	DMA_OP_COPY,
	DMA_OP_SPR_TRANSFER,
} DmaOp;

typedef struct DmaCmd
{
	DmaOp op;
	uint8_t stride;
	uint8_t len_1;
	uint8_t len_2;
	uint8_t src_1;  // Used as data for DMA fill.
	uint8_t src_2;
	uint8_t src_3;
	uint32_t ctrl;
} DmaCmd;

// DMA queue ring buffer.
static volatile uint16_t s_md_dma_write_pos;
static volatile uint16_t s_md_dma_read_pos;
static DmaCmd s_dma_q[DMA_QUEUE_DEPTH];
static uint16_t s_dma_spr_pos;
static DmaCmd s_dma_spr_cmd[2];

void md_dma_init(void)
{
	s_md_dma_read_pos = 0;
	s_md_dma_write_pos = 0;
	s_dma_spr_pos = 0;
	s_dma_spr_cmd[0].op = DMA_OP_NONE;
	s_dma_spr_cmd[1].op = DMA_OP_NONE;
}

// Calculate required register values for a transfer
static inline void enqueue_int(DmaOp op, uint32_t bus, uint16_t dest,
                               uint32_t src, uint16_t n, uint16_t stride)
{
	// Disable interrupts for this critical section.
	const uint16_t ints_enabled = md_sys_get_ints_enabled();
	md_sys_di();
	MD_SYS_BARRIER();

	DmaCmd *cmd;
	if (op == DMA_OP_SPR_TRANSFER)
	{
		if (s_dma_spr_pos >= 2) goto finish;
		cmd = &s_dma_spr_cmd[s_dma_spr_pos];
		s_dma_spr_pos++;
	}
	else
	{
		cmd = &s_dma_q[s_md_dma_write_pos];
		s_md_dma_write_pos = (s_md_dma_write_pos + 1) % DMA_QUEUE_DEPTH;
		if (s_md_dma_write_pos == s_md_dma_read_pos) goto finish;
	}

	cmd->op = op;
	cmd->stride = stride;
	cmd->len_1 = n & 0xFF;
	cmd->len_2 = (n >> 8) & 0xFF;

	switch (op)
	{
		default:
			return;

		case DMA_OP_TRANSFER:
		case DMA_OP_SPR_TRANSFER:
			src = src >> 1;
			cmd->src_1 = src & 0xFF;
			src = src >> 8;
			cmd->src_2 = src & 0xFF;
			src = src >> 8;
			cmd->src_3 = src & 0x7F;
			break;

		case DMA_OP_FILL:
			cmd->src_1 = src & 0xFF;
			cmd->src_3 = VDP_DMA_SRC_FILL;
			break;

		case DMA_OP_COPY:
			cmd->src_1 = src & 0xFF;
			cmd->src_2 = (src >> 8) & 0xFF;
			cmd->src_3 = VDP_DMA_SRC_COPY;
			break;
	}

	cmd->ctrl = VDP_CTRL_DMA_BIT | VDP_CTRL_ADDR(dest) | bus;

finish:
	// Re-enable ints if they were enabled before.
	MD_SYS_BARRIER();
	if (ints_enabled) md_sys_ei();
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
	// based on SGDK's DMA validation
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
	              dest, (uint32_t )src, words, stride);
}

void md_dma_transfer_cram(uint16_t dest, const void *src, uint16_t words,
                         uint16_t stride)
{
	md_dma_enqueue(DMA_OP_TRANSFER, VDP_CTRL_CRAM_WRITE,
	              dest, (uint32_t )src, words, stride);
}

void md_dma_transfer_vsram(uint16_t dest, const void *src, uint16_t words,
                          uint16_t stride)
{
	md_dma_enqueue(DMA_OP_TRANSFER, VDP_CTRL_VSRAM_WRITE,
	              dest, (uint32_t )src, words, stride);
}

void md_dma_transfer_spr_vram(uint16_t dest, const void *src, uint16_t words,
                             uint16_t stride)
{
	md_dma_enqueue(DMA_OP_SPR_TRANSFER, VDP_CTRL_VRAM_WRITE,
	              dest, (uint32_t )src, words, stride);
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

static inline void process_cmd(DmaCmd *cmd)
{
	MD_SYS_BARRIER();

	md_vdp_set_autoinc(cmd->stride);
	md_vdp_set_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);

	md_vdp_set_reg(VDP_DMALEN1, cmd->len_1);
	md_vdp_set_reg(VDP_DMALEN2, cmd->len_2);
	MD_SYS_BARRIER();

	switch (cmd->op)
	{
		default:
			break;
	
		case DMA_OP_FILL:
			md_vdp_set_reg(VDP_DMASRC3, cmd->src_3);
			VDPPORT_CTRL32 = cmd->ctrl;
			VDPPORT_DATA = (cmd->src_1 << 8) | (cmd->src_1);
			break;

		case DMA_OP_SPR_TRANSFER:
		case DMA_OP_TRANSFER:
		case DMA_OP_COPY:
			md_vdp_set_reg(VDP_DMASRC1, cmd->src_1);
			md_vdp_set_reg(VDP_DMASRC2, cmd->src_2);
			MD_SYS_BARRIER();
			md_vdp_set_reg(VDP_DMASRC3, cmd->src_3);
			VDPPORT_CTRL32 = cmd->ctrl;
			break;
	}

	MD_SYS_BARRIER();
	md_vdp_wait_dma();
	MD_SYS_BARRIER();
	md_vdp_clear_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);
}

void md_dma_process(void)
{
	md_vdp_wait_dma();
	MD_SYS_BARRIER();

	const uint16_t ints_enabled = md_sys_get_ints_enabled();
	md_sys_di();
	md_sys_z80_bus_req();
	MD_SYS_BARRIER();

	// Process single high-priority slot first.
	if (s_dma_spr_cmd[0].op == DMA_OP_SPR_TRANSFER)
	{
		process_cmd(&s_dma_spr_cmd[0]);
		s_dma_spr_cmd[0].op = DMA_OP_NONE;
	}
	if (s_dma_spr_cmd[1].op == DMA_OP_SPR_TRANSFER)
	{
		process_cmd(&s_dma_spr_cmd[1]);
		s_dma_spr_cmd[1].op = DMA_OP_NONE;
	}

	s_dma_spr_pos = 0;


	// Process all queued tranfers.
	while (s_md_dma_read_pos != s_md_dma_write_pos)
	{
		DmaCmd *cmd = &s_dma_q[s_md_dma_read_pos];
		s_md_dma_read_pos = (s_md_dma_read_pos + 1) % DMA_QUEUE_DEPTH;
		process_cmd(cmd);
	}

	MD_SYS_BARRIER();
	md_sys_z80_bus_release();
	if (ints_enabled) md_sys_ei();
}
