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
static uint16_t s_palette[256];
static uint16_t s_dirty = 0xFFFF;
static uint16_t s_initialized = 0;
#endif  // MDK_TARGET_C2

// Functions to translate between color formats
// -----------------------------------------------------------------------------

static inline uint16_t rgb555_from_rgb333(uint16_t in_rgb333)
{
	const uint16_t cred = (in_rgb333 & 0x00E);
	const uint16_t cgreen = (in_rgb333 & 0x0E0) << 1;
	const uint16_t cblue = (in_rgb333 & 0xE00) << 2;
	uint16_t value = cred | cgreen | cblue;
	static const uint16_t upper_two_bit_mask = 0x6318;  // 0b0110001100011000.
;	value |= (value & upper_two_bit_mask) >> 3;  // Upper 2 bits into LSBs.
	return value;
}

static inline uint16_t rgb333_from_rgb555(uint16_t in_rgb555)
{
	const uint16_t cred = (in_rgb555 & 0x1C) >> 1;
	const uint16_t cgreen = (in_rgb555 & (0x1C << 5)) >> 2;
	const uint16_t cblue = (in_rgb555 & (0x1C << 10)) >> 3;
	uint16_t value = cred | cgreen | cblue;
	return value;
}

// Individual color set functions
// -----------------------------------------------------------------------------

void md_pal_set(uint8_t idx, uint16_t val)
{
	s_palette[idx % ARRAYSIZE(s_palette)] = val;
}

// MD-native RGB333 format
void md_pal_set_rgb333(uint8_t idx, uint16_t val)
{
#ifndef MDK_TARGET_C2
	s_palette[idx % ARRAYSIZE(s_palette)] = val;
#else
	s_palette[idx % ARRAYSIZE(s_palette)] = rgb555_from_rgb333(val);
#endif
}

// C2-native RGB555 format
void md_pal_set_rgb555(uint8_t idx, uint16_t val)
{
#ifdef MDK_TARGET_C2
	s_palette[idx % ARRAYSIZE(s_palette)] = val;
#else
	s_palette[idx % ARRAYSIZE(s_palette)] = rgb333_from_rgb555(val);
#endif
}

// Color upload functions
// -----------------------------------------------------------------------------

// MD-native RGB333 format
void md_pal_upload_rgb333(uint16_t dest, const void *source, uint16_t count)
{
#ifndef MDK_TARGET_C2
	md_pal_upload(dest, source, count);
#else
	const uint16_t *source_16 = (const uint16_t *)source;
	for (uint16_t i = 0; i < count; i++)
	{
		s_palette[dest++] = rgb555_from_rgb333(*source_16++);
	}
#endif
}

// C2-native RGB555 format
void md_pal_upload_rgb555(uint16_t dest, const void *source, uint16_t count)
{
#ifdef MDK_TARGET_C2
	md_pal_upload(dest, source, count);
#else
	const uint16_t *source_16 = (const uint16_t *)source;
	for (uint16_t i = 0; i < count; i++)
	{
		s_palette[dest++] = rgb333_from_rgb555(*source_16++);
	}
#endif
}

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

#else

// =============================================================================
// System C/C2 implementation, as a memory copy into color RAM
// =============================================================================

void md_pal_set_sysc_map(uint16_t bg_pos, uint16_t spr_pos)
{
	volatile uint8_t *prot = (volatile uint8_t *)SYSC_PROTECTION_LOC_SECURITY;
	*prot = (bg_pos & 0x03) | ((spr_pos | 0x03) << 2);
}

void md_pal_poll(void)
{
	if (!s_initialized)
	{
		md_ioc_set_pal_bank(0);
		md_vdp_set_reg_bit(VDP_MODESET4, VDP_MODESET4_EXT_CBUS_EN);
		md_pal_set_sysc_map(0, 0);  // Default to an MD-compatible mode.
		s_initialized = 1;
	}

	md_vdp_clear_reg_bit(VDP_MODESET3, VDP_MODESET3_CBUS_VDP_CTRL);
	uint16_t test_bit = 0x0001;
	uint16_t source_idx_l = 0;
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
			volatile uint32_t *cram32 = (volatile uint32_t *)CRAM_SYSTEMC_LOC_BASE;
			volatile uint32_t *src32 = (uint32_t *)s_palette;
			cram32 += source_idx_l;
			src32 += source_idx_l;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
			*cram32++ = *src32++;
		}
		test_bit = test_bit << 1;
		source_idx_l += 8;
	}
	md_vdp_set_reg_bit(VDP_MODESET3, VDP_MODESET3_CBUS_VDP_CTRL);
	s_dirty = 0;
}



#endif  // MD_TARGET_C2
