#pragma once

# A1 - CSPR blob
.set	CSPR_NAME, 0x00
.set	CSPR_PALETTE, 0x10
.set	CSPR_REF_COUNT, 0x30
.set	CSPR_SPR_LIST_OFFSET, 0x32
.set	CSPR_TILE_DATA_OFFSET, 0x36
.set	CSPR_REFS, 0x40

# A2 - Frame ref
.set	REF_SPR_COUNT, 0
.set	REF_SPR_LIST_OFFSET, 2
.set	REF_TILE_SRC_OFFSET, 4
.set	REF_TILE_WORDS, 6

# A1 - Sprite data
.set	SPR_DY, 0
.set	SPR_SIZE, 2
.set	SPR_TILE, 4
.set	SPR_DX, 6
.set	SPR_FDY, 8
.set	SPR_RESERVED1, 10
.set	SPR_RESERVED2, 12
.set	SPR_FDX, 14
