#include "tiles.h"

// GBA VRAM does not support 8-bit writes — all tile data must use u16 or u32.
// 4bpp tile = 32 bytes = 16 u16 words.
// Each u16 word encodes 4 pixels: bits[3:0]=px0, bits[7:4]=px1, bits[11:8]=px2, bits[15:12]=px3.

// Fill a tile with a repeated color (4bpp: 0x0-0xF)
static void fill_tile(volatile u16 *cbb, int tile_idx, u8 color) {
    volatile u16 *dst = cbb + tile_idx * 16;
    u16  px  = (color & 0xF);
    u16  word = px | (px << 4) | (px << 8) | (px << 12); // 4 pixels
    for (int i = 0; i < 16; i++) dst[i] = word;
}

// Write a tile from an array of 16 u16 words
static void write_tile(volatile u16 *cbb, int tile_idx, const u16 *data) {
    volatile u16 *dst = cbb + tile_idx * 16;
    for (int i = 0; i < 16; i++) dst[i] = data[i];
}

void tiles_load_placeholders(void) {
    // ── BG Palette ────────────────────────────────────────────────────────
    // Pal 0 (sky): 0=backdrop, 1=sky light, 2=sky mid, 3=sky dark
    BG_PAL[0*16 + 0] = RGB15(16, 22, 31);
    BG_PAL[0*16 + 1] = RGB15(20, 26, 31);
    BG_PAL[0*16 + 2] = RGB15(14, 20, 29);
    BG_PAL[0*16 + 3] = RGB15( 8, 14, 24);

    // Pal 1 (slope): 1=bright snow, 2=snow shadow, 3=mountain grey, 4=mountain dark
    BG_PAL[1*16 + 0] = RGB15(16, 22, 31);
    BG_PAL[1*16 + 1] = RGB15(31, 31, 31); // white snow
    BG_PAL[1*16 + 2] = RGB15(24, 26, 31); // snow shadow
    BG_PAL[1*16 + 3] = RGB15(18, 18, 20); // mountain grey
    BG_PAL[1*16 + 4] = RGB15(12, 12, 14); // mountain dark
    BG_PAL[1*16 + 5] = RGB15(20, 22, 28); // slope line

    // Pal 2 (trees): 1=dark pine, 2=mid pine, 3=snow on tree
    BG_PAL[2*16 + 0] = RGB15(16, 22, 31);
    BG_PAL[2*16 + 1] = RGB15( 2, 12,  2);
    BG_PAL[2*16 + 2] = RGB15( 4, 18,  4);
    BG_PAL[2*16 + 3] = RGB15(28, 29, 31);

    // ── Tile graphics (CBB 0, 4bpp, u16 writes) ───────────────────────────
    u16 *cbb0 = CBB(0);

    // Tile 0: sky light (all pixels = pal color 1)
    fill_tile(cbb0, TILE_SKY_LIGHT, 1);

    // Tile 1: sky dark (all pixels = pal color 3)
    fill_tile(cbb0, TILE_SKY_DARK, 3);

    // Tile 2: mountain — top rows dark grey (4), bottom medium grey (3)
    {
        static const u16 mt[16] = {
            0x4444, 0x4444, // rows 0-1: dark top
            0x4444, 0x4444, // rows 2-3
            0x3333, 0x3333, // rows 4-5: medium grey
            0x4334, 0x3433, // rows 6-7: mixed
            0x3333, 0x3333, // rows 8-9
            0x3333, 0x3333, // rows 10-11
            0x3333, 0x3333, // rows 12-13
            0x3333, 0x3333, // rows 14-15
        };
        write_tile(cbb0, TILE_MOUNTAIN, mt);
    }

    // Tile 3: bright snow (all pixels = pal color 1)
    fill_tile(cbb0, TILE_SNOW, 1);

    // Tile 4: snow with shadow stripes (alternating colors 1 and 2)
    {
        static const u16 snow_shadow[16] = {
            0x1111, 0x1111, // row 0-1: bright
            0x2222, 0x2222, // row 2-3: shadow
            0x1111, 0x1111,
            0x2222, 0x2222,
            0x1111, 0x1111,
            0x2222, 0x2222,
            0x1111, 0x1111,
            0x2222, 0x2222,
        };
        write_tile(cbb0, TILE_SNOW_SHADOW, snow_shadow);
    }

    // Tile 5: tree silhouette (pal 2: dark/mid green, snow cap)
    {
        static const u16 tree[16] = {
            0x3333, 0x0330, // snow cap
            0x2222, 0x0220, // upper green
            0x2222, 0x2222,
            0x1122, 0x2211, // mid
            0x1111, 0x1111, // trunk
            0x1111, 0x1111,
            0x0000, 0x0000, // gap
            0x0000, 0x0000,
        };
        write_tile(cbb0, TILE_TREE, tree);
    }
}
