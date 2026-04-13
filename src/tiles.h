#pragma once
#include "gba.h"

// Placeholder tile indices (4bpp, palette 0)
#define TILE_SKY_LIGHT   0
#define TILE_SKY_DARK    1
#define TILE_MOUNTAIN    2
#define TILE_SNOW        3
#define TILE_SNOW_SHADOW 4
#define TILE_TREE        5

// Palette slot assignments (16-color palettes)
#define PAL_SKY   0   // BG pal 0: sky tones
#define PAL_SLOPE 1   // BG pal 1: snow, mountain tones
#define PAL_TREE  2   // BG pal 2: tree greens

// Load all placeholder tile data and palette into VRAM/PAL RAM.
// Real assets will replace this when we have sprites.
void tiles_load_placeholders(void);
