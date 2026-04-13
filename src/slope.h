#pragma once
#include "gba.h"

// Slope scroller: 3 BG layers with parallax.
//   BG0 (prio 3): Sky — barely scrolls
//   BG1 (prio 2): Mountain silhouette — scrolls at 25% speed
//   BG2 (prio 1): Snow slope — scrolls at full speed (main layer)
//
// Scroll position is tracked in fixed-point (16.16).
// slope_update() advances the scroll and writes to HW registers.

#define SLOPE_SPEED_DEFAULT INT_TO_FX(1)   // 1 pixel/frame baseline
#define SLOPE_SPEED_MAX     INT_TO_FX(4)   // max at full hype

typedef struct {
    fixed scroll_x;         // current horizontal scroll (FX 16.16)
    fixed speed;            // current scroll speed (FX 16.16)
    int   ground_y;         // pixel row where snow surface sits (BG2)
} Slope;

void slope_init(Slope *s);
void slope_set_speed(Slope *s, fixed speed);
void slope_update(Slope *s);
void slope_build_maps(void);
