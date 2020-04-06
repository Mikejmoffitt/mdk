/* md-toolchain DMA control functions
Michael Moffitt 2018 */

#include "md/dma.h"
#include "md/vdp.h"
#include "md/sys.h"

#define DMA_CMD_OP_TRANSFER 0x000
#define DMA_CMD_OP_FILL     0x100
#define DMA_CMD_OP_COPY     0x200

// Max bytes to transfer per frame during vblank, copied from md.squee.co
#define DMA_H40_TRANSFER_PER_LINE           198
#define DMA_H32_TRANSFER_PER_LINE           161

#define DMA_NTSC_H32_TRANSFER_BANDWIDTH     6118
#define DMA_NTSC_H40_TRANSFER_BANDWIDTH     7524
#define DMA_PAL_H32_TRANSFER_BANDWIDTH      14329
#define DMA_PAL_H40_TRANSFER_BANDWIDTH      17622
#define DMA_PAL_H32_V30_TRANSFER_BANDWIDTH  11753
#define DMA_PAL_H40_V30_TRANSFER_BANDWIDTH  14454

// This should be a power of 2, since it is used with the modulo operator.
#define DMA_QUEUE_DEPTH 128

#define DMA_BUDGET_INF 65536

typedef struct DmaCmd
{
	uint32_t src;
	uint16_t type; // op mask 0x300; bus mask 0x3
	uint16_t dest;
	uint16_t n;
	uint16_t stride;
} DmaCmd;

static volatile uint16_t dma_budget;
// DMA queue ring buffer.
static volatile uint16_t dma_queue_write_pos;
static volatile uint16_t dma_queue_read_pos;
static DmaCmd dma_queue[DMA_QUEUE_DEPTH];

static inline void dma_q_enqueue(uint16_t op, uint16_t bus, uint16_t dest,
                                 uint32_t src, uint16_t n, uint16_t stride)
{
	const uint16_t ints_enabled = sys_get_ints_enabled();
	sys_di();
	DmaCmd *cmd = &dma_queue[dma_queue_write_pos];
	dma_queue_write_pos = (dma_queue_write_pos + 1) % DMA_QUEUE_DEPTH;
	if (dma_queue_write_pos == dma_queue_read_pos) return;
	cmd->src = src;
	cmd->type = (bus | op);
	cmd->dest = dest;
	cmd->n = n;
	cmd->stride = stride;
	sys_ei();
	if (ints_enabled) sys_ei();
}

static inline uint32_t calc_dma_cost(DmaCmd *cmd)
{
	// Bandwidth is in *bytes* for VRAM transfers, words otherwise, so VRAM
	// cost is doubled as n specifies words.
	return ((cmd->type & 0xFF) == DMA_OP_BUS_VRAM) ? (cmd->n << 1) : (cmd->n);
}

static void internal_dma_queue_proc(uint16_t budget_rem)
{
	const uint16_t disp_disabled = !(vdp_get_reg(VDP_MODESET2) &
	                                 VDP_MODESET2_DISP_EN);
	while (dma_queue_read_pos != dma_queue_write_pos)
	{
		DmaCmd *cmd = &dma_queue[dma_queue_read_pos];
		dma_queue_read_pos = (dma_queue_read_pos + 1) % DMA_QUEUE_DEPTH;
		uint16_t cost = calc_dma_cost(cmd);

		dma_set_stride(cmd->stride);

		switch(cmd->type & 0xFF00)
		{
			default:
				continue;
			case DMA_CMD_OP_TRANSFER:
				dma_transfer(cmd->type, cmd->dest, (const void *)(cmd->src),
				             cmd->n);
				break;
			case DMA_CMD_OP_COPY:
				dma_copy(cmd->type, cmd->dest, (uint16_t)(cmd->src), cmd->n);
				break;
			case DMA_CMD_OP_FILL:
				dma_fill(cmd->type, cmd->dest, (uint16_t)(cmd->src), cmd->n);
				break;
		}

		if (disp_disabled || budget_rem == DMA_Q_BUDGET_UNLIMITED) continue;

		// TODO: If cost is greater than budget_rem, split the DMA up
		// and do what is possible, and then queue the remainder for the
		// next vblank and exit. For now, it allows it to go slightly
		// over budget.
		// If the display is disabled, budget is entirely ignored.
		if (cost >= budget_rem)
		{
			break;
		}

		budget_rem -= cost;
	}
}

// Schedule a DMA for next vblank from 68K mem to VRAM
void dma_q_transfer_vram(uint16_t dest, const void *src, uint16_t n,
                         uint16_t stride)
{
	dma_q_enqueue(DMA_CMD_OP_TRANSFER, DMA_OP_BUS_VRAM,
	              dest, (uint32_t )src, n, stride);
}

void dma_q_transfer_cram(uint16_t dest, const void *src, uint16_t n,
                         uint16_t stride)
{
	dma_q_enqueue(DMA_CMD_OP_TRANSFER, DMA_OP_BUS_CRAM,
	              dest, (uint32_t )src, n, stride);
}

void dma_q_transfer_vsram(uint16_t dest, const void *src, uint16_t n,
                          uint16_t stride)
{
	dma_q_enqueue(DMA_CMD_OP_TRANSFER, DMA_OP_BUS_VSRAM,
	              dest, (uint32_t )src, n, stride);
}

// Schedule a DMA for next vblank to fill n words at dest with val.
void dma_q_fill_vram(uint16_t dest, uint16_t val, uint16_t n, uint16_t stride)
{
	dma_q_enqueue(DMA_CMD_OP_FILL, DMA_OP_BUS_VRAM, dest, val, n, stride);
}

// Schedule a DMA for next vblank to copy n words from VRAM src to VRAM dest.
void dma_q_copy_vram(uint16_t dest, uint16_t src, uint16_t n, uint16_t stride)
{
	dma_q_enqueue(DMA_CMD_OP_COPY, DMA_OP_BUS_VRAM, dest, src, n, stride);
}

// Configure the DMA queue vblank transfer budget for max_words per vblank.
// DMA_Q_BUDGET_AUTO uses a calculation based on the current mode.
void dma_q_set_budget(uint16_t max_bytes)
{
	if (max_bytes == DMA_Q_BUDGET_AUTO)
	{
		if (vdp_get_raster_width() == 320)
		{
			if (sys_is_pal())
			{
				dma_budget = vdp_get_raster_height() == 240 ?
				             DMA_PAL_H40_V30_TRANSFER_BANDWIDTH :
				             DMA_PAL_H40_TRANSFER_BANDWIDTH;
			}
			else
			{
				dma_budget = DMA_NTSC_H40_TRANSFER_BANDWIDTH;
			}
		}
		else
		{
			if (sys_is_pal())
			{
				dma_budget = vdp_get_raster_height() == 240 ?
				             DMA_PAL_H32_V30_TRANSFER_BANDWIDTH:
				             DMA_PAL_H32_TRANSFER_BANDWIDTH;
			}
			else
			{
				dma_budget = DMA_NTSC_H32_TRANSFER_BANDWIDTH;
			}
		}
	}
	else
	{
		dma_budget = max_bytes;
	}

	if (dma_budget < DMA_Q_BUDGET_UNLIMITED) dma_budget = dma_budget / 2;
}

void dma_q_process(void)
{
	internal_dma_queue_proc(dma_budget);
}

// Finish all remaining DMAs in the queue.
void dma_q_complete(void)
{
	const uint16_t ints_enabled = sys_get_ints_enabled();
	sys_di();
	internal_dma_queue_proc(DMA_Q_BUDGET_UNLIMITED);
	if (ints_enabled) sys_ei();
}

void dma_q_flush(void)
{
	dma_queue_read_pos = 0;
	dma_queue_write_pos = 0;
}

void dma_fill(uint16_t bus, uint16_t dest, uint16_t val, uint16_t n)
{
	uint32_t ctrl_mask = VDP_CTRL_DMA_BIT;
	const uint16_t ints_enabled = sys_get_ints_enabled();

	switch(bus & 0x0003)
	{
		default:
			return;
		case DMA_OP_BUS_VRAM:
			ctrl_mask |= VDP_CTRL_VRAM_WRITE;
			break;
		case DMA_OP_BUS_VSRAM:
			ctrl_mask |= VDP_CTRL_VSRAM_WRITE;
			break;
		case DMA_OP_BUS_CRAM:
			ctrl_mask |= VDP_CTRL_CRAM_WRITE;
			break;
	}

	sys_di();
	vdp_wait_dma();
	vdp_set_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);

	// Configure DMA length
	vdp_set_reg(VDP_DMALEN1, n & 0xFF);
	vdp_set_reg(VDP_DMALEN2, (n >> 8) & 0xFF);

	vdp_set_reg(VDP_DMASRC3, VDP_DMA_SRC_FILL);

	VDPPORT_CTRL32 = (ctrl_mask | VDP_CTRL_ADDR(dest));
	VDPPORT_DATA = val << 8;
	// TODO: Do we care about Z80 here?
	if (ints_enabled) sys_ei();
}

void dma_copy(uint16_t bus, uint16_t dest, uint16_t src, uint16_t n)
{
	uint32_t ctrl_mask = VDP_CTRL_DMA_BIT;
	const uint16_t ints_enabled = sys_get_ints_enabled();

	switch(bus & 0x0003)
	{
		default:
			return;
		case DMA_OP_BUS_VRAM:
			ctrl_mask |= VDP_CTRL_VRAM_WRITE;
			break;
		case DMA_OP_BUS_VSRAM:
			ctrl_mask |= VDP_CTRL_VSRAM_WRITE;
			break;
		case DMA_OP_BUS_CRAM:
			ctrl_mask |= VDP_CTRL_CRAM_WRITE;
			break;
	}

	vdp_wait_dma();
	vdp_set_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);

	// Configure DMA length
	vdp_set_reg(VDP_DMALEN1, n & 0xFF);
	vdp_set_reg(VDP_DMALEN2, (n >> 8) & 0xFF);

	vdp_set_reg(VDP_DMASRC1, src & 0xFF);
	src = src >> 8;
	vdp_set_reg(VDP_DMASRC2, src & 0xFF);
	vdp_set_reg(VDP_DMASRC3, VDP_DMA_SRC_COPY);

	VDPPORT_CTRL32 = (ctrl_mask | VDP_CTRL_ADDR(dest));
	// TODO: Do we care about Z80 here?
	if (ints_enabled) sys_ei();
}

void dma_transfer(uint16_t bus, uint16_t dest, const void *src, uint16_t n)
{
	uint32_t ctrl_mask = VDP_CTRL_DMA_BIT;
	uint32_t transfer_src = (uint32_t)src;
	uint32_t transfer_limit;
	uint16_t transfer_len = n;
	const uint16_t ints_enabled = sys_get_ints_enabled();

	// check that the source address + length won't cross a 128KIB boundary
	// based on SGDK's DMA validation
	transfer_limit = 0x20000 - (transfer_src & 0x1FFFF);

	// If the tranfer will cross the 128KiB boundary, transfer the latter
	// half first, then truncate the transfer's length to fill the rest.
	if (transfer_len > (transfer_limit >> 1))
	{
		dma_transfer(bus, dest + transfer_limit,
		             (const void *)(transfer_src + transfer_limit),
		             n - (transfer_limit >> 1));
		transfer_len = transfer_limit >> 1;
	}

	switch(bus & 0x0003)
	{
		default:
			return;
		case DMA_OP_BUS_VRAM:
			ctrl_mask |= VDP_CTRL_VRAM_WRITE;
			break;
		case DMA_OP_BUS_VSRAM:
			ctrl_mask |= VDP_CTRL_VSRAM_WRITE;
			break;
		case DMA_OP_BUS_CRAM:
			ctrl_mask |= VDP_CTRL_CRAM_WRITE;
			break;
	}

	sys_di();

	vdp_wait_dma();
	vdp_set_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);

	// Configure DMA length
	vdp_set_reg(VDP_DMALEN1, transfer_len & 0xFF);
	vdp_set_reg(VDP_DMALEN2, (transfer_len >> 8) & 0xFF);

	transfer_src = transfer_src >> 1;
	vdp_set_reg(VDP_DMASRC1, transfer_src & 0xFF);
	transfer_src = transfer_src >> 8;
	vdp_set_reg(VDP_DMASRC2, transfer_src & 0xFF);
	transfer_src = transfer_src >> 8;
	vdp_set_reg(VDP_DMASRC3, transfer_src & 0x7F);

	sys_z80_bus_req();

	// Set write address
	VDPPORT_CTRL32 = (ctrl_mask | VDP_CTRL_ADDR(dest));

	vdp_clear_reg_bit(VDP_MODESET2, VDP_MODESET2_DMA_EN);

	sys_z80_bus_release();
	if (ints_enabled) sys_ei();

}
