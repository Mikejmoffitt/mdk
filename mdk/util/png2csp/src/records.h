// Functions and data related to recording the XSP structures to disk.
#ifndef RECORDS_H
#define RECORDS_H

#include <stdbool.h>
#include <stdint.h>
#include "types.h"

#define RECORD_MAX_TILE_COUNT 0x10000
#define RECORD_MAX_SPR_COUNT 0x1000
#define RECORD_MAX_REF_COUNT 0x1000

// Returns true if initialization was successful.
bool record_init(void);

// Writes to disk.
void record_complete(const char *spr_name, const char *fname);

void record_palette(const uint16_t *pal_data);
void record_spr(int dx, int dy, int w, int h, int tile, int flip_dx, int flip_dy);
void record_ref(int spr_count, int spr_index, int tile_index, int tile_count);
void record_tiles(const uint8_t *src, int count);

int record_get_spr_count(void);
int record_get_ref_count(void);
int record_get_tile_count(void);

#endif  // RECORDS_H
