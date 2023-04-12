// Function to claim and erase a region from a sprite.

#ifndef CLAIM_H
#define CLAIM_H

#include <stdint.h>

typedef enum ClaimSize
{
	CLAIM_SIZE_NONE = 0,
	CLAIM_SIZE_1x1,
	CLAIM_SIZE_1x2,
	CLAIM_SIZE_1x3,
	CLAIM_SIZE_1x4,
	CLAIM_SIZE_2x1,
	CLAIM_SIZE_2x2,
	CLAIM_SIZE_2x3,
	CLAIM_SIZE_2x4,
	CLAIM_SIZE_3x1,
	CLAIM_SIZE_3x2,
	CLAIM_SIZE_3x3,
	CLAIM_SIZE_3x4,
	CLAIM_SIZE_4x1,
	CLAIM_SIZE_4x2,
	CLAIM_SIZE_4x3,
	CLAIM_SIZE_4x4,
} ClaimSize;

int tiles_for_claim_size(ClaimSize size);
int tile_w_for_claim_size(ClaimSize size);
int tile_h_for_claim_size(ClaimSize size);

// Finds a sprite to clip out of imgdat.
// Returns CLAIM_SIZE_NONE if imgdat is empty.
ClaimSize claim(const uint8_t *imgdat,
                int iw, int ih,
                int sx, int sy, int sw, int sh,
                int *col, int *row);

#endif  // CLAIM_H
