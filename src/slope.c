#include "slope.h"
#include "tiles.h"
#include <string.h>

// ── BG layout ────────────────────────────────────────────────────────────────
// CBB 0 → tile graphics (shared by all BGs)
// SBB 28 → BG0 map (sky)
// SBB 29 → BG1 map (mountains)
// SBB 30 → BG2 map (slope)
//
// GBA screen: 240×160. BG map: 32×32 tiles of 8×8px = 256×256px.
// We scroll horizontally; the map wraps naturally via HW.

#define SKY_SBB   28
#define MTN_SBB   29
#define SLOPE_SBB 30

// Tile map entry (4bpp): bits[9:0]=tile index, bit[10]=hflip, bit[11]=vflip,
// bits[15:12]=palette number
#define MAP_ENTRY(tile, pal) ((u16)((tile) | ((pal) << 12)))

// Screen-block helpers
static u16 *sky_map   = NULL;
static u16 *mtn_map   = NULL;
static u16 *slope_map = NULL;

void slope_build_maps(void) {
    sky_map   = SBB(SKY_SBB);
    mtn_map   = SBB(MTN_SBB);
    slope_map = SBB(SLOPE_SBB);

    // GBA screen is 240px wide = 30 tiles; map is 32 tiles wide (wraps).
    // 160px tall = 20 tiles; map is 32 tiles tall.

    // ── BG0: Sky ─────────────────────────────────────────────────────────
    // Upper portion: light sky tiles. Near horizon (row 14+): darker.
    for (int row = 0; row < 32; row++) {
        u16 tile = (row < 14) ? MAP_ENTRY(TILE_SKY_LIGHT, PAL_SKY)
                               : MAP_ENTRY(TILE_SKY_DARK,  PAL_SKY);
        for (int col = 0; col < 32; col++)
            sky_map[row * 32 + col] = tile;
    }

    // ── BG1: Mountain silhouette ──────────────────────────────────────────
    // Fill sky above mountains, mountain tiles in a jagged band (rows 10-16)
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 32; col++) {
            // Mountains: zigzag height based on column parity
            int mtn_top = 10 + ((col * 3) & 3); // 0..3 variation, no division
            u16 tile;
            if (row < mtn_top)
                tile = MAP_ENTRY(TILE_SKY_LIGHT, PAL_SKY);     // sky above mountain
            else if (row < mtn_top + 4)
                tile = MAP_ENTRY(TILE_MOUNTAIN, PAL_SLOPE);     // mountain body
            else
                tile = MAP_ENTRY(TILE_SNOW, PAL_SLOPE);         // snow below
            mtn_map[row * 32 + col] = tile;
        }
    }

    // ── BG2: Snow slope ───────────────────────────────────────────────────
    // Sky above row 14, solid snow below. Trees scattered every ~6 cols.
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 32; col++) {
            u16 tile;
            if (row < 14)
                tile = MAP_ENTRY(TILE_SKY_LIGHT, PAL_SKY);
            else if (row == 14)
                tile = MAP_ENTRY(TILE_SNOW_SHADOW, PAL_SLOPE);  // slope surface line
            else if (row < 18 && (col % 6 == 0))
                tile = MAP_ENTRY(TILE_TREE, PAL_TREE);           // tree on slope side
            else
                tile = MAP_ENTRY(TILE_SNOW, PAL_SLOPE);
            slope_map[row * 32 + col] = tile;
        }
    }

    // ── BG register setup ─────────────────────────────────────────────────
    // All BGs use CBB 0 for tile data.
    REG_BG0CNT = BG_PRIO(3) | BG_CBB(0) | BG_SBB(SKY_SBB)   | BG_4BPP | BG_SIZE_32x32;
    REG_BG1CNT = BG_PRIO(2) | BG_CBB(0) | BG_SBB(MTN_SBB)   | BG_4BPP | BG_SIZE_32x32;
    REG_BG2CNT = BG_PRIO(1) | BG_CBB(0) | BG_SBB(SLOPE_SBB) | BG_4BPP | BG_SIZE_32x32;
}

void slope_init(Slope *s) {
    s->scroll_x  = 0;
    s->speed     = SLOPE_SPEED_DEFAULT;
    s->ground_y  = 14 * 8; // pixel row 112
}

void slope_set_speed(Slope *s, fixed speed) {
    s->speed = speed;
    if (s->speed > SLOPE_SPEED_MAX) s->speed = SLOPE_SPEED_MAX;
    if (s->speed < INT_TO_FX(0))   s->speed = INT_TO_FX(0);
}

void slope_update(Slope *s) {
    s->scroll_x += s->speed;
    int sx = FX_TO_INT(s->scroll_x);

    // BG0 (sky): very slow, 1/8 of slope speed
    REG_BG0HOFS = sx >> 3;
    REG_BG0VOFS = 0;

    // BG1 (mountains): ~1/4 of slope speed (shift instead of divide)
    REG_BG1HOFS = sx >> 2;
    REG_BG1VOFS = 0;

    // BG2 (slope): full speed
    REG_BG2HOFS = sx;
    REG_BG2VOFS = 0;
}
