/* mdk palette support functions for MD
Michael Moffitt 2018-2022 */

#include "md/pal.h"
#include "md/dma.h"
#include "md/macro.h"
#include "md/vdp.h"
#include "md/ioc.h"

#include <string.h>

#ifndef MDK_TARGET_C2

#define MDK_PAL_DIRTY_MASK_FULL 0x000F
uint16_t g_palette[16 * 4];
static uint16_t s_dirty = MDK_PAL_DIRTY_MASK_FULL;
#else
// Two sets, between BG and Sprites of:
//   Four banks of:
//     Four palette lines, each with:
//       Sixteen colors.
// Four global banks exist, but I do not feel they are particularly useful when
// four banks already exist within each color section.
uint16_t g_palette[16 * 4 * 4 * 2];
#define MDK_PAL_DIRTY_MASK_FULL 0x1FFFF
static uint32_t s_dirty = MDK_PAL_DIRTY_MASK_FULL;
static uint8_t s_prot_reg_cache;
#endif  // MDK_TARGET_C2

// Interface
// -----------------------------------------------------------------------------
void md_pal_mark_dirty(uint16_t first_index, uint16_t count)
{
	const uint16_t first_line = (first_index / 16) % (ARRAYSIZE(g_palette) / 16);
	const uint16_t last_line = (count / 16) % (ARRAYSIZE(g_palette) / 16);
	if (last_line < first_line)
	{
		s_dirty |= MDK_PAL_DIRTY_MASK_FULL;
		return;
	}

	uint32_t dirty_mask = (1 << first_line);
	for (uint16_t i = 0; i <= last_line - first_line; i++)
	{
		s_dirty |= dirty_mask;
		dirty_mask = dirty_mask << 1;
	}
}


// Individual color set functions
// -----------------------------------------------------------------------------

void md_pal_set(uint16_t idx, uint16_t val)
{
	idx = idx % ARRAYSIZE(g_palette);
	g_palette[idx] = val;
	s_dirty |= (1 << (idx >> 4));
}

// Color upload functions
// -----------------------------------------------------------------------------

// Upload as-is.
void md_pal_upload(uint16_t dest, const void *source, uint16_t count)
{
	if (dest + count > ARRAYSIZE(g_palette)) return;
	md_pal_mark_dirty(dest, count);
	memcpy(&g_palette[dest], source, count * sizeof(uint16_t));
}

#ifndef MDK_TARGET_C2
// =============================================================================
// Megadrive implementation, using DMA
// =============================================================================
void md_pal_poll(void)
{
	// The s_dirty bitfield is broken down case by case here because
	// consecutive palette lines can be uploaded in one DMA transfer.
	switch (s_dirty)
	{
		case 0x0:  // ....
			break;

		case 0x1:  // 0...
			md_dma_transfer_cram(0, &g_palette[0], 16, 2);
			break;

		case 0x2:  // .1..
			md_dma_transfer_cram(32, &g_palette[16], 16, 2);
			break;

		case 0x3:  // 01..
			md_dma_transfer_cram(0, &g_palette[0], 32, 2);
			break;

		case 0x4:  // ..2.
			md_dma_transfer_cram(64, &g_palette[32], 16, 2);
			break;

		case 0x5:  // 0.2.
			md_dma_transfer_cram(0, &g_palette[0], 16, 2);
			md_dma_transfer_cram(64, &g_palette[32], 16, 2);
			break;

		case 0x6:  // .12.
			md_dma_transfer_cram(32, &g_palette[16], 16, 2);
			md_dma_transfer_cram(64, &g_palette[32], 16, 2);
			break;

		case 0x7:  // 012.
			md_dma_transfer_cram(0, &g_palette[0], 48, 2);
			break;

		case 0x8:  // ...3
			md_dma_transfer_cram(96, &g_palette[48], 16, 2);
			break;

		case 0x9:  // 0..3
			md_dma_transfer_cram(96, &g_palette[48], 16, 2);
			md_dma_transfer_cram(0, &g_palette[0], 16, 2);
			break;

		case 0xA:  // .1.3
			md_dma_transfer_cram(96, &g_palette[48], 16, 2);
			md_dma_transfer_cram(32, &g_palette[16], 16, 2);
			break;

		case 0xB:  // 01.3
			md_dma_transfer_cram(96, &g_palette[48], 16, 2);
			md_dma_transfer_cram(0, &g_palette[0], 32, 2);
			break;

		case 0xC:  // ..23
			md_dma_transfer_cram(64, &g_palette[48], 32, 2);
			break;

		case 0xD:  // 0.23
			md_dma_transfer_cram(64, &g_palette[48], 32, 2);
			md_dma_transfer_cram(0, &g_palette[0], 16, 2);
			break;

		case 0xE:  // .123
			md_dma_transfer_cram(32, &g_palette[48], 48, 2);
			break;

		case 0xF:  // 0123
			md_dma_transfer_cram(0, &g_palette[0], 64, 2);
			break;
	}
	s_dirty = 0;
}

void md_pal_init(void)
{
	memset(g_palette, 0, sizeof(g_palette));
	s_dirty = 0x000F;
}

#else

// =============================================================================
// System C/C2 implementation, as a memory copy into color RAM
// =============================================================================

// Select between banks 0 - 3 for sprites and backgrounds.
void md_pal_set_spr_bank(uint16_t bank)
{
	s_prot_reg_cache &= ~(0x03 << 2);
	s_prot_reg_cache |= (bank & 0x03) << 2;
	volatile uint8_t *prot = (volatile uint8_t *)SYSC_PROTECTION_LOC_SECURITY;
	*prot = s_prot_reg_cache;
}

void md_pal_set_bg_bank(uint16_t bank)
{
	s_prot_reg_cache &= ~(0x03);
	s_prot_reg_cache |= (bank & 0x03);
	volatile uint8_t *prot = (volatile uint8_t *)SYSC_PROTECTION_LOC_SECURITY;
	*prot = s_prot_reg_cache;
}

void md_pal_poll(void)
{
	md_vdp_set_cbus_cpu_mux(0);
	uint32_t test_bit = 0x00000001;
	volatile uint32_t *cram32 = (volatile uint32_t *)CRAM_SYSTEMC_LOC_BASE;
	volatile uint32_t *src32 = (uint32_t *)g_palette;
	for (uint16_t i = 0; i < ARRAYSIZE(g_palette) / 16; i++)
	{
		if (s_dirty & test_bit)
		{
			uint16_t *cram = (uint16_t *)CRAM_SYSTEMC_LOC_BASE;
			memcpy(&cram[i * 16], &g_palette[i * 16], sizeof(uint16_t) * 16);
		}
		test_bit = test_bit << 1;
		cram32 += 8;
		src32 += 8;
	}
	md_vdp_set_cbus_cpu_mux(1);
	s_dirty = 0;
}

void md_pal_init(void)
{
	volatile uint8_t *prot = (volatile uint8_t *)SYSC_PROTECTION_LOC_SECURITY;
	*prot = 0x00;
	MD_SYS_BARRIER();
	md_pal_set_spr_bank(0);
	md_pal_set_bg_bank(0);
	memset(g_palette, 0, sizeof(g_palette));
	s_dirty = 0x0001FFFF;
}

#endif  // MD_TARGET_C2
