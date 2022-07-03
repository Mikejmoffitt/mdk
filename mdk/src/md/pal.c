/* md-toolchain palette support functions
Michael Moffitt 2018 */
#include "md/pal.h"
#include "md/dma.h"

static uint16_t s_palette[64];
static uint16_t s_dirty = 0xF;

void md_pal_set(uint8_t idx, uint16_t val)
{
	s_palette[idx % 64] = val;
}

void md_pal_upload(uint8_t dest, const void *source, uint8_t len)
{
	if (dest >= 64) return;
	const uint16_t pal_line = (dest >> 4) % 4;
	s_dirty |= (1 << pal_line);
	const uint16_t *source_16 = (const uint16_t *)source;
	for (int16_t i = 0; i < len; i++)
	{
		s_palette[dest++] = *source_16++;
	}
}

void md_pal_poll(void)
{
	switch (s_dirty)
	{
		case 0x0:
			break;

		case 0x1:
			dma_q_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0x2:
			dma_q_transfer_cram(32, &s_palette[16], 16, 2);
			break;

		case 0x3:
			dma_q_transfer_cram(0, &s_palette[0], 32, 2);
			break;

		case 0x4:
			dma_q_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x5:
			dma_q_transfer_cram(0, &s_palette[0], 16, 2);
			dma_q_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x6:
			dma_q_transfer_cram(32, &s_palette[16], 16, 2);
			dma_q_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x7:
			dma_q_transfer_cram(0, &s_palette[0], 48, 2);
			break;

		case 0x8:
			dma_q_transfer_cram(96, &s_palette[48], 16, 2);
			break;

		case 0x9:
			dma_q_transfer_cram(96, &s_palette[48], 16, 2);
			dma_q_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0xA:
			dma_q_transfer_cram(96, &s_palette[48], 16, 2);
			dma_q_transfer_cram(32, &s_palette[16], 16, 2);
			break;

		case 0xB:
			dma_q_transfer_cram(96, &s_palette[48], 16, 2);
			dma_q_transfer_cram(0, &s_palette[0], 32, 2);
			break;

		case 0xC:
			dma_q_transfer_cram(64, &s_palette[48], 32, 2);
			break;

		case 0xD:
			dma_q_transfer_cram(64, &s_palette[48], 32, 2);
			dma_q_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0xE:
			dma_q_transfer_cram(32, &s_palette[48], 48, 2);
			break;

		case 0xF:
			dma_q_transfer_cram(0, &s_palette[0], 64, 2);
			break;
	}
	s_dirty = 0;
}

#undef PAL_DIRTY_0
#undef PAL_DIRTY_1
#undef PAL_DIRTY_2
#undef PAL_DIRTY_3
