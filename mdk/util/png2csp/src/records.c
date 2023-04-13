#include "records.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "claim.h"
#include "endian.h"

//
// Types
//

//
// REF
//

/*
2   spr quantity used
2   spr list index * sizeof(spr def)
2   tile data start offset (within tile data block)
2   tile count * 16 (words, for DMA, pre-multiplied by VDP tile size)
*/

typedef struct RecordRef
{
	int spr_count;
	int spr_index;
	int tile_index;  // Offset within tile bank.
	int tile_count;
} RecordRef;


//
// SPR
//

/*
2   relative Y offset (signed word)
2   sprite size attribute field (other bits 0)
2   tile id
2   relative X offset (signed word)
2   flip X offset
2   flip Y offset
4   reserved
*/

typedef struct RecordSpr
{
	int dx, dy;  // Relative offsets.
	int w, h;  // In tiles.
	int tile;  // From first tile of tile bank.
	int flip_dx, flip_dy;
} RecordSpr;

//
// Storage
//

// "REF" structures, metasprites.
static RecordRef s_ref_dat[RECORD_MAX_REF_COUNT];
static int s_ref_count = 0;
// "SPR" structures, hardware sprite draw data.
static RecordSpr s_spr_dat[RECORD_MAX_SPR_COUNT];
static uint32_t s_spr_count = 0;
// Tile data.
static uint8_t s_tile_dat[RECORD_MAX_TILE_COUNT * TBYTES];
static int s_tile_count = 0;
// Palette data.
static uint16_t s_pal_dat[PALSIZE];

//
// Functions for recording output file data
//

static void write_ref(const RecordRef *ref, FILE *f)
{
	fwrite_uint16be(ref->spr_count, f);
	fwrite_uint16be(ref->spr_index * 16, f);  // * sizeof(spr_def)
	fwrite_uint16be(ref->tile_index * TBYTES, f);  // Offset within tile data.
	fwrite_uint16be(ref->tile_count * (TBYTES / 2), f);  // DMA size in words.
}

static void write_spr(const RecordSpr *spr, FILE *f)
{
	fwrite_int16be(spr->dy, f);
	const uint16_t sizebits = ((spr->h-1) << 8) | ((spr->w-1) << 10);
	fwrite_uint16be(sizebits, f);
	fwrite_uint16be(spr->tile, f);
	fwrite_int16be(spr->dx, f);
	fwrite_int16be(spr->flip_dy, f);
	fwrite_uint16be(0, f);
	fwrite_uint16be(0, f);
	fwrite_int16be(spr->flip_dx, f);
}

//
// Record functions
//

bool record_init(void)
{
	s_tile_count = 0;
	s_spr_count = 0;
	s_ref_count = 0;
	return true;
}

void record_complete(const char *spr_name, const char *fname)
{
	FILE *f = fopen(fname, "wb");
	if (!f)
	{
		fprintf(stderr, "Couldn't open \"%s\" for writing.\n", fname);
		return;
	}

/*
Binary blob format:
-------------------
$00     16  bank name (null-terminated C string)
$10     32  palette data
$30     2   sprite ref count (available images)
$32     4   spr list offset (from start of file)
$36     4   tile data offset (from start of file)
$3A     2   tile count
$3C     4   reserved
$40     ... ref list
...     ... spr list data (length variable; referened by header)
...     ... tile data (length variable; referenced by header)
*/

	// Write the bank name.
	char spr_name_out[16];
	strncpy(spr_name_out, spr_name, sizeof(spr_name_out)-1);
	spr_name_out[15] = '\0';
	fwrite(spr_name_out, 1, sizeof(spr_name_out), f);
	// Palette data.
	for (int i = 0; i < PALSIZE; i++)
	{
		fwrite_uint16be(s_pal_dat[i], f);
	}
	// Sprite ref count.
	fwrite_uint16be(s_ref_count, f);
	// Offsets (written later).
	const long spr_list_offset_loc = ftell(f);
	fwrite_uint32be(0, f);
	const long tile_data_offset_loc = ftell(f);
	fwrite_uint32be(0, f);
	// Tile count.
	fwrite_uint16be(s_tile_count, f);
	// Padding.
	fwrite_uint16be(0, f);
	fwrite_uint16be(0, f);
	// The ref list.
	for (int i = 0; i < s_ref_count; i++)
	{
		write_ref(&s_ref_dat[i], f);
	}
	// The spr list.
	const long spr_list_loc = ftell(f);
	for (int i = 0; i < s_spr_count; i++)
	{
		write_spr(&s_spr_dat[i], f);
	}
	// The tile data.
	const long tile_data_loc = ftell(f);
	fwrite(s_tile_dat, TBYTES, s_tile_count, f);

	// Compute offsets and record them.
	fseek(f, spr_list_offset_loc, SEEK_SET);
	fwrite_uint32be((uint32_t)(spr_list_loc), f);
	fseek(f, tile_data_offset_loc, SEEK_SET);
	fwrite_uint32be((uint32_t)(tile_data_loc), f);

	fclose(f);
}

void record_palette(const uint16_t *pal_data)
{
	memcpy(s_pal_dat, pal_data, sizeof(uint16_t) * PALSIZE);
}

void record_ref(int spr_count, int spr_index, int tile_index, int tile_count)
{
	if (s_ref_count >= RECORD_MAX_REF_COUNT)
	{
		fprintf(stderr, "Can't record any more REF data!\n");
		return;
	}
	RecordRef *ref = &s_ref_dat[s_ref_count];
	s_ref_count++;

	ref->spr_count = spr_count;
	ref->spr_index = spr_index;
	ref->tile_index = tile_index;
	ref->tile_count = tile_count;
}

void record_spr(int dx, int dy, int w, int h, int tile, int flip_dx, int flip_dy)
{
	if (s_spr_count >= RECORD_MAX_SPR_COUNT)
	{
		fprintf(stderr, "Can't record any more SPR data!\n");
		return;
	}
	RecordSpr *spr = &s_spr_dat[s_spr_count];
	s_spr_count++;

	spr->dx = dx;
	spr->dy = dy;
	spr->w = w;
	spr->h = h;
	spr->tile = tile;
	spr->flip_dx = flip_dx;
	spr->flip_dy = flip_dy;
}

void record_tiles(const uint8_t *src, int count)
{
	if (s_tile_count + count > RECORD_MAX_TILE_COUNT)
	{
		fprintf(stderr, "Cannot record any more tiles!\n");
		return;
	}
	memcpy(&s_tile_dat[s_tile_count * TBYTES], src, TBYTES * count);
	s_tile_count += count;
}

int record_get_tile_count(void)
{
	return s_tile_count;
}

int record_get_spr_count(void)
{
	return s_spr_count;
}

int record_get_ref_count(void)
{
	return s_ref_count;
}
