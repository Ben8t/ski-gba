#include "skier.h"
#include <string.h>

// OAM entry format (4 u16 words per object):
//   attr0: Y pos [7:0], shape [15:14], color [13], ...
//   attr1: X pos [8:0], size [15:14], hflip [12], vflip [13]
//   attr2: tile index [9:0], priority [11:10], palette [15:12]
//   attr3: (affine only)

#define OBJ_SHAPE_SQUARE 0
#define OBJ_SIZE_16x16   1   // with SQUARE shape = 16x16

// OBJ sprite tile base: in Mode 0 OBJ tiles start at 0x06010000
// In 4bpp, tile 0 = 0x06010000, each tile = 32 bytes
#define OBJ_TILE_BASE ((u16*)0x06010000)

// Skier uses OBJ tile 0 (placeholder: solid colored 16x16 block)
// A 16x16 sprite = 4 tiles in 1D mapping (2x2 grid of 8x8 tiles)
#define SKIER_OBJ_TILE 0
#define SKIER_OBJ_PAL  0     // OBJ palette 0

// Gravity constant in FX per frame^2 (accelerates downward)
#define GRAVITY INT_TO_FX(1) / 4  // 0.25 px/frame^2 — tunable

static int _just_landed = 0;

void skier_load_sprite(void) {
    // OBJ palette 0: color 0 = transparent, color 1 = skier red placeholder
    OBJ_PAL[0] = 0;                      // transparent
    OBJ_PAL[1] = RGB15(28, 8, 4);        // red (placeholder skier body)
    OBJ_PAL[2] = RGB15(4, 12, 28);       // blue (placeholder ski)
    OBJ_PAL[3] = RGB15(31, 28, 8);       // yellow (helmet)

    // Write 4 tiles (16x16 = 2x2 tiles, 4bpp):
    // Simple pattern: body = color 1, ski outline = color 2
    u8 *td = (u8*)OBJ_TILE_BASE;
    // Fill all 4 tiles with color 1 (skier body)
    for (int i = 0; i < 4 * 32; i++) td[i] = 0x11;
    // Top 2 tiles: add helmet color (color 3) on top row
    for (int col = 0; col < 4; col++) td[col] = 0x33;
    // Bottom 2 tiles (tiles 2&3): add ski color bottom row
    u8 *t2 = td + 2*32; // tile 2
    for (int col = 0; col < 4; col++) t2[28 + col] = 0x22; // last row of tile 2
}

void skier_init(Skier *sk, int ground_y) {
    sk->screen_x  = 60;
    sk->ground_y  = ground_y;
    sk->screen_y  = ground_y - 16; // sprite top = ground_y - sprite height
    sk->vy        = 0;
    sk->state     = SKIER_STATE_SKIING;
    sk->airtime   = 0;
    sk->jump_height = 0;
    _just_landed  = 0;
}

void skier_launch(Skier *sk, int height_px, int airtime_frames) {
    if (sk->state != SKIER_STATE_SKIING && sk->state != SKIER_STATE_APPROACH)
        return;
    // Calculate initial upward velocity so peak = height_px, duration = airtime_frames/2
    // Using h = v0*t - 0.5*g*t^2, peak at t = v0/g → h = v0^2/(2g)
    // Simpler: set vy so that after airtime/2 frames the skier is at peak
    sk->jump_height = height_px;
    sk->airtime     = 0;
    // vy (up) = height / (airtime/2) pixels per frame.
    // Use libgcc integer division — acceptable at jump init (once per jump).
    int half = airtime_frames >> 1;
    if (half < 1) half = 1;
    sk->vy = -((INT_TO_FX(height_px)) / half); // negative = upward
    sk->state = SKIER_STATE_AIRBORNE;
}

void skier_update(Skier *sk) {
    _just_landed = 0;

    switch (sk->state) {
    case SKIER_STATE_SKIING:
    case SKIER_STATE_APPROACH:
        sk->screen_y = sk->ground_y - 16;
        sk->vy = 0;
        break;

    case SKIER_STATE_AIRBORNE:
        sk->vy += GRAVITY;       // gravity pulls down
        sk->screen_y += FX_TO_INT(sk->vy);
        sk->airtime++;

        // Check landing: skier returned to or below ground
        if (sk->screen_y >= sk->ground_y - 16) {
            sk->screen_y = sk->ground_y - 16;
            sk->vy = 0;
            sk->state = SKIER_STATE_LANDING;
            _just_landed = 1;
        }
        break;

    case SKIER_STATE_LANDING:
        // Landing persists for a couple frames (for QTE / scoring window)
        // Caller transitions back to SKIING after processing
        sk->screen_y = sk->ground_y - 16;
        break;
    }
}

void skier_draw(const Skier *sk) {
    // Write OAM entry 0 (first sprite)
    volatile u16 *obj = OAM;

    // attr0: Y[7:0], no rotation, no mosaic, 4bpp, square shape
    obj[0] = (sk->screen_y & 0xFF) | (OBJ_SHAPE_SQUARE << 14);
    // attr1: X[8:0], no flip, 16x16 size
    obj[1] = (sk->screen_x & 0x1FF) | (OBJ_SIZE_16x16 << 14);
    // attr2: tile 0, priority 0, OBJ pal 0
    obj[2] = SKIER_OBJ_TILE | (SKIER_OBJ_PAL << 12);
    // attr3: unused (non-affine)
    obj[3] = 0;
}

int skier_just_landed(const Skier *sk) {
    (void)sk;
    return _just_landed;
}
