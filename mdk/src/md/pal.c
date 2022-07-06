/* mdk palette support functions for MD
Michael Moffitt 2018-2022 */

#include "md/pal.h"
#include "md/dma.h"
#include "md/macro.h"
#include "md/vdp.h"
#include "md/ioc.h"

#ifndef MDK_TARGET_C2
static uint16_t s_palette[64];
static uint16_t s_dirty = 0x000F;
#else
// Two sets, between BG and Sprites of:
//   Four banks of:
//     Four palette lines, each with:
//       Sixteen colors.
// Four global banks exist, but I do not feel they are particularly useful when
// four banks already exist within each color section.
static uint16_t s_palette[16 * 4 * 4 * 2];
static uint32_t s_dirty = 0x0001FFFF;
static uint8_t s_prot_reg_cache;
#endif  // MDK_TARGET_C2

// Individual color set functions
// -----------------------------------------------------------------------------

void md_pal_set(uint16_t idx, uint16_t val)
{
	idx = idx % ARRAYSIZE(s_palette);
	s_palette[idx] = val;
	s_dirty |= (1 << (idx >> 4));
}

// Color upload functions
// -----------------------------------------------------------------------------

// Upload as-is.
void md_pal_upload(uint16_t dest, const void *source, uint16_t count)
{
	const uint16_t pal_line = (dest >> 4) % (ARRAYSIZE(s_palette) / 16);
	s_dirty |= (1 << pal_line);

	const uint16_t *source_16 = (const uint16_t *)source;
#ifdef MD_PAL_STANDARD_COPY_LOOP
	for (uint16_t i = 0; i < count; i++)
	{
		s_palette[dest++] = *source_16++;
	}
#else
	// TODO: Profile this duff's device against a standard copy loop.
	// Let's make sure it's not just a code boondoggle! It is 2022
	uint16_t n = (count + 15) / 16;
	switch (count % 16)
	{
		case 0:
			do
			{
				s_palette[dest++] = *source_16++;
		case 15:
				s_palette[dest++] = *source_16++;
		case 14:
				s_palette[dest++] = *source_16++;
		case 13:
				s_palette[dest++] = *source_16++;
		case 12:
				s_palette[dest++] = *source_16++;
		case 11:
				s_palette[dest++] = *source_16++;
		case 10:
				s_palette[dest++] = *source_16++;
		case 9:
				s_palette[dest++] = *source_16++;
		case 8:
				s_palette[dest++] = *source_16++;
		case 7:
				s_palette[dest++] = *source_16++;
		case 6:
				s_palette[dest++] = *source_16++;
		case 5:
				s_palette[dest++] = *source_16++;
		case 4:
				s_palette[dest++] = *source_16++;
		case 3:
				s_palette[dest++] = *source_16++;
		case 2:
				s_palette[dest++] = *source_16++;
		case 1:
				s_palette[dest++] = *source_16++;
			} while (--n > 0);
	}
#endif  // MD_PAL_STANDARD_COPY_LOOP
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
		case 0x0:
			break;

		case 0x1:
			md_dma_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0x2:
			md_dma_transfer_cram(32, &s_palette[16], 16, 2);
			break;

		case 0x3:
			md_dma_transfer_cram(0, &s_palette[0], 32, 2);
			break;

		case 0x4:
			md_dma_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x5:
			md_dma_transfer_cram(0, &s_palette[0], 16, 2);
			md_dma_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x6:
			md_dma_transfer_cram(32, &s_palette[16], 16, 2);
			md_dma_transfer_cram(64, &s_palette[32], 16, 2);
			break;

		case 0x7:
			md_dma_transfer_cram(0, &s_palette[0], 48, 2);
			break;

		case 0x8:
			md_dma_transfer_cram(96, &s_palette[48], 16, 2);
			break;

		case 0x9:
			md_dma_transfer_cram(96, &s_palette[48], 16, 2);
			md_dma_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0xA:
			md_dma_transfer_cram(96, &s_palette[48], 16, 2);
			md_dma_transfer_cram(32, &s_palette[16], 16, 2);
			break;

		case 0xB:
			md_dma_transfer_cram(96, &s_palette[48], 16, 2);
			md_dma_transfer_cram(0, &s_palette[0], 32, 2);
			break;

		case 0xC:
			md_dma_transfer_cram(64, &s_palette[48], 32, 2);
			break;

		case 0xD:
			md_dma_transfer_cram(64, &s_palette[48], 32, 2);
			md_dma_transfer_cram(0, &s_palette[0], 16, 2);
			break;

		case 0xE:
			md_dma_transfer_cram(32, &s_palette[48], 48, 2);
			break;

		case 0xF:
			md_dma_transfer_cram(0, &s_palette[0], 64, 2);
			break;
	}
	s_dirty = 0;
}

void md_pal_init(void)
{
	for (uint16_t i = 0; i < ARRAYSIZE(s_palette); i++)
	{
		s_palette[i] = 0;
	}
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
	md_vdp_clear_reg_bit(VDP_MODESET3, VDP_MODESET3_CBUS_VDP_CTRL);
	uint32_t test_bit = 0x00000001;
	volatile uint32_t *cram32 = (volatile uint32_t *)CRAM_SYSTEMC_LOC_BASE;
	volatile uint32_t *src32 = (uint32_t *)s_palette;
	for (uint16_t i = 0; i < ARRAYSIZE(s_palette) / 16; i++)
	{
		// TODO: Consider asm here - wouldn't this be so much nicer if we just
		// cleared the relevant bit and checked the Z flag?
		if (s_dirty & test_bit)
		{
			// Copy a whole palette line as 32-bit ints because
			// 1) It is guaranteed on 68000 that the palette, made of 16-bit
			//    words, is word-aligned (otherwise a uint16_t * would not work)
			// 2) 32-bit accesses on 16-bit alignment is absolutely okay
			// 3) It is a wee bit faster (skip every other instruction fetch)
			volatile uint32_t *dest = cram32;
			volatile uint32_t *src = src32;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
		}
		test_bit = test_bit << 1;
		cram32 += 8;
		src32 += 8;
	}
	md_vdp_set_reg_bit(VDP_MODESET3, VDP_MODESET3_CBUS_VDP_CTRL);
	s_dirty = 0;
}

void md_pal_init(void)
{
	volatile uint8_t *prot = (volatile uint8_t *)SYSC_PROTECTION_LOC_SECURITY;
	*prot = 0x00;
	MD_SYS_BARRIER();
	md_ioc_set_pal_bank(0);
	md_vdp_set_reg_bit(VDP_MODESET4, VDP_MODESET4_EXT_CBUS_EN);
	md_pal_set_spr_bank(0);
	md_pal_set_bg_bank(0);
	for (uint16_t i = 0; i < ARRAYSIZE(s_palette); i++)
	{
		s_palette[i] = 0;
	}
	s_dirty = 0x0001FFFF;
}

#endif  // MD_TARGET_C2
