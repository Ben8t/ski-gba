#include "tiles.h"
#include <string.h>

// Each 4bpp tile = 32 bytes (8x8 pixels, 4 bits per pixel, 2 pixels per byte)
// Pixel value 0 = transparent for OBJ, palette[0] for BG
// Format: each u32 = 8 pixels (2 rows of 4 pixels each in 4bpp)

// Helper: fill a tile with a single color index repeated
static void fill_tile(u16 *tile_base, int tile_idx, u8 color_idx) {
    u8 px = (color_idx & 0xF) | ((color_idx & 0xF) << 4);
    u8 *dst = (u8*)(tile_base + tile_idx * 16); // 32 bytes per tile, tile_base is u16*
    for (int i = 0; i < 32; i++) dst[i] = px;
}

// Helper: fill a tile row-by-row from a 64-byte array of nibbles
static void write_tile(u16 *tile_base, int tile_idx, const u8 *data) {
    u8 *dst = (u8*)(tile_base + tile_idx * 16);
    memcpy(dst, data, 32);
}

void tiles_load_placeholders(void) {
    // ── BG Palette ────────────────────────────────────────────────────────
    // Pal 0 (sky): 0=transparent/bg, 1=sky light, 2=sky mid, 3=sky dark
    BG_PAL[0*16 + 0] = RGB15(16, 22, 31); // sky fill (light blue)
    BG_PAL[0*16 + 1] = RGB15(20, 26, 31); // lightest sky
    BG_PAL[0*16 + 2] = RGB15(14, 20, 29); // mid sky
    BG_PAL[0*16 + 3] = RGB15( 8, 14, 24); // dark sky (horizon)

    // Pal 1 (slope): 0=bg same as sky, 1=bright snow, 2=snow shadow, 3=mountain grey, 4=mountain dark, 5=slope line
    BG_PAL[1*16 + 0] = RGB15(16, 22, 31);
    BG_PAL[1*16 + 1] = RGB15(31, 31, 31); // pure white snow
    BG_PAL[1*16 + 2] = RGB15(24, 26, 31); // snow in shadow (blue-white)
    BG_PAL[1*16 + 3] = RGB15(18, 18, 20); // mountain rock grey
    BG_PAL[1*16 + 4] = RGB15(12, 12, 14); // mountain dark
    BG_PAL[1*16 + 5] = RGB15(20, 22, 28); // slope line

    // Pal 2 (trees): 1=dark green, 2=mid green, 3=snow on tree
    BG_PAL[2*16 + 0] = RGB15(16, 22, 31);
    BG_PAL[2*16 + 1] = RGB15( 2, 12,  2); // dark pine
    BG_PAL[2*16 + 2] = RGB15( 4, 18,  4); // mid pine
    BG_PAL[2*16 + 3] = RGB15(28, 29, 31); // snow on tree

    // ── Tile graphics (CBB 0, 4bpp) ───────────────────────────────────────
    u16 *cbb0 = CBB(0);

    // Tile 0: sky light (solid color 1 from pal 0)
    fill_tile(cbb0, TILE_SKY_LIGHT, 1);

    // Tile 1: sky dark (solid color 3, used near horizon)
    fill_tile(cbb0, TILE_SKY_DARK, 3);

    // Tile 2: mountain — grey with a triangular silhouette feel
    // Simple: top half dark (3), bottom half light (2) — reads as mountain ridge
    {
        u8 t[32];
        for (int row = 0; row < 8; row++) {
            u8 c = (row < 4) ? 0x44 : 0x33; // dark then medium grey (pal 1)
            // encode as 4bpp: each byte = 2 pixels
            for (int col = 0; col < 4; col++) t[row*4 + col] = c;
        }
        write_tile(cbb0, TILE_MOUNTAIN, t);
        // Override: pack palette slot 1 into nibbles (pal select done in map entry)
        // Actually for 4bpp the nibble is the color within the palette
        // mountain uses pal 1: color 3 = grey, color 4 = dark
        u8 mt[32];
        for (int row = 0; row < 8; row++) {
            u8 c;
            if (row < 3)      c = 0x44; // dark grey top
            else if (row < 5) c = 0x33; // medium grey mid
            else              c = 0x43; // mixed bottom
            for (int col = 0; col < 4; col++) mt[row*4 + col] = c;
        }
        write_tile(cbb0, TILE_MOUNTAIN, mt);
    }

    // Tile 3: bright snow (solid white, pal 1 color 1)
    fill_tile(cbb0, TILE_SNOW, 1);

    // Tile 4: snow with shadow stripe (pal 1 colors 1 and 2)
    {
        u8 t[32];
        for (int row = 0; row < 8; row++) {
            u8 c = (row % 4 < 2) ? 0x11 : 0x22;
            for (int col = 0; col < 4; col++) t[row*4+col] = c;
        }
        write_tile(cbb0, TILE_SNOW_SHADOW, t);
    }

    // Tile 5: simplified tree silhouette (pal 2)
    {
        // Simple triangle: narrow top, wide bottom
        const u8 tree_rows[8] = {
            0x00, // ....
            0x02, // .X..  (center green)
            0x22, // XX..
            0x22, // XX..
            0x12, // XY.. (dark base)
            0x11, // XX
            0x30, // snow cap top
            0x00,
        };
        u8 t[32];
        for (int row = 0; row < 8; row++) {
            // Spread the pattern across 4 bytes (8 pixels)
            u8 c = tree_rows[row];
            for (int col = 0; col < 4; col++) t[row*4+col] = c;
        }
        write_tile(cbb0, TILE_TREE, t);
    }
}
