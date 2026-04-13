#include "skier.h"

// OBJ sprite layout:
//   OBJ tile data: 0x06010000 (OBJ VRAM in Mode 0)
//   OAM: 0x07000000, 4 u16 words per object
//   attr0: Y[7:0], obj mode[9:8], mosaic[12], colors[13], shape[15:14]
//   attr1: X[8:0], transform[13:9], hflip[12], vflip[13], size[15:14]
//   attr2: tile[9:0], priority[11:10], palette[15:12]
//
// 16x16 OBJ = shape SQUARE + size 1 = uses 4 consecutive tiles (2x2 grid in 1D map)

#define OBJ_SHAPE_SQUARE  (0 << 14)
#define OBJ_SIZE_16x16    (1 << 14)

#define SKIER_OBJ_TILE 0
#define SKIER_OBJ_PAL  0

// Gravity: FX 16.16 per frame^2
#define GRAVITY (INT_TO_FX(1) >> 2)  // 0.25 px/frame^2

static int _just_landed = 0;

void skier_load_sprite(void) {
    // OBJ palette 0 — must write as u16 (palette RAM supports 16-bit writes)
    OBJ_PAL[0] = 0;                    // transparent
    OBJ_PAL[1] = RGB15(28, 8,  4);    // red body
    OBJ_PAL[2] = RGB15( 4, 12, 28);   // blue ski
    OBJ_PAL[3] = RGB15(31, 28, 8);    // yellow helmet

    // OBJ VRAM: 16x16 sprite = 4 tiles (tiles 0-3 in 1D mapping)
    // GBA VRAM requires u16 writes (no byte writes)
    u16 *ot = (u16*)0x06010000;

    // Tile 0 (top-left): helmet (color 3) on top row, body (color 1) rest
    for (int i = 0; i <  2; i++) ot[i]    = 0x3333; // helmet row
    for (int i = 2; i < 16; i++) ot[i]    = 0x1111; // body

    // Tile 1 (top-right): same
    for (int i = 0; i <  2; i++) ot[16+i] = 0x3333;
    for (int i = 2; i < 16; i++) ot[16+i] = 0x1111;

    // Tile 2 (bottom-left): body + ski at bottom
    for (int i = 0; i < 14; i++) ot[32+i] = 0x1111;
    for (int i = 14; i < 16; i++) ot[32+i] = 0x2222; // ski

    // Tile 3 (bottom-right): same
    for (int i = 0; i < 14; i++) ot[48+i] = 0x1111;
    for (int i = 14; i < 16; i++) ot[48+i] = 0x2222;
}

void skier_init(Skier *sk, int ground_y) {
    sk->screen_x  = 60;
    sk->ground_y  = ground_y;
    sk->screen_y  = ground_y - 16;
    sk->vy        = 0;
    sk->state     = SKIER_STATE_SKIING;
    sk->airtime   = 0;
    sk->jump_height = 0;
    _just_landed  = 0;
}

void skier_launch(Skier *sk, int height_px, int airtime_frames) {
    if (sk->state != SKIER_STATE_SKIING && sk->state != SKIER_STATE_APPROACH)
        return;
    sk->jump_height = height_px;
    sk->airtime     = 0;
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
        sk->vy += GRAVITY;
        sk->screen_y += FX_TO_INT(sk->vy);
        sk->airtime++;
        if (sk->screen_y >= sk->ground_y - 16) {
            sk->screen_y = sk->ground_y - 16;
            sk->vy = 0;
            sk->state = SKIER_STATE_LANDING;
            _just_landed = 1;
        }
        break;

    case SKIER_STATE_LANDING:
        sk->screen_y = sk->ground_y - 16;
        break;
    }
}

void skier_draw(const Skier *sk) {
    volatile u16 *obj = OAM;
    // attr0: Y pos, square shape, 4bpp (bit 13 = 0)
    obj[0] = (u16)(sk->screen_y & 0xFF) | OBJ_SHAPE_SQUARE;
    // attr1: X pos, 16x16 size
    obj[1] = (u16)(sk->screen_x & 0x1FF) | OBJ_SIZE_16x16;
    // attr2: tile 0, priority 0, palette 0
    obj[2] = SKIER_OBJ_TILE | (SKIER_OBJ_PAL << 12);
    obj[3] = 0;
}

int skier_just_landed(const Skier *sk) {
    (void)sk;
    return _just_landed;
}
