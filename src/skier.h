#pragma once
#include "gba.h"

// Skier sprite: OBJ layer, sits on the slope surface.
// For now uses a solid 16x16 colored box as placeholder.
// Horizontal position is fixed (screen center-left ~60px).
// Vertical position tracks ground_y during normal skiing,
// then follows a parabolic arc during jumps.

typedef enum {
    SKIER_STATE_SKIING,
    SKIER_STATE_APPROACH,   // kicker visible, about to jump
    SKIER_STATE_AIRBORNE,
    SKIER_STATE_LANDING,
} SkierState;

typedef struct {
    s16        screen_x;    // fixed screen X (stays ~60)
    s16        screen_y;    // current screen Y
    s16        ground_y;    // Y to return to on landing
    fixed      vy;          // vertical velocity (FX, negative = up)
    SkierState state;
    int        airtime;     // frames since launch
    int        jump_height; // peak height in pixels (set at launch)
} Skier;

void skier_init(Skier *sk, int ground_y);
void skier_launch(Skier *sk, int height_px, int airtime_frames);
void skier_update(Skier *sk);
void skier_draw(const Skier *sk);
void skier_load_sprite(void);

// Returns 1 if skier just entered LANDING state this frame
int skier_just_landed(const Skier *sk);
