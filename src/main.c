#include "gba.h"
#include "tiles.h"
#include "slope.h"
#include "skier.h"

// ── Input ─────────────────────────────────────────────────────────────────────
#define REG_KEYINPUT (*(volatile u16*)0x04000130)
#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_L      (1 << 9)
#define KEY_R      (1 << 8)

#define key_held(k)    (!(REG_KEYINPUT & (k)))

static u16 _prev_keys = 0xFFFF;
#define key_pressed(k) ((_prev_keys & (k)) && !(REG_KEYINPUT & (k)))
static void input_update(void) { _prev_keys = REG_KEYINPUT; }

// ── Hype meter ────────────────────────────────────────────────────────────────
static int hype = 64; // 0..256

static void hype_add(int delta) {
    hype += delta;
    if (hype > 256) hype = 256;
    if (hype < 0)   hype = 0;
}

static fixed hype_to_speed(void) {
    // 0.5 + (hype/256) * 3.5 → shift to avoid division
    return INT_TO_FX(1) + (INT_TO_FX(3) * hype >> 8);
}

int main(void) {
    REG_DISPCNT = DISPCNT_MODE0 | DISPCNT_BG0_ON | DISPCNT_BG1_ON |
                  DISPCNT_BG2_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D;

    tiles_load_placeholders();
    slope_build_maps();

    Slope slope;
    slope_init(&slope);

    skier_load_sprite();
    Skier skier;
    skier_init(&skier, 112);

    while (1) {
        vsync();
        input_update();

        slope_set_speed(&slope, hype_to_speed());
        slope_update(&slope);

        if (key_pressed(KEY_A) && skier.state == SKIER_STATE_SKIING) {
            int height  = 32 + (hype * 24 >> 8);
            int airtime = 50 + (hype * 30 >> 8);
            skier_launch(&skier, height, airtime);
        }
        if (key_held(KEY_L)) hype_add(-2);
        if (key_held(KEY_R)) hype_add(+2);

        if (skier.state == SKIER_STATE_LANDING) {
            hype_add(+20);
            skier.state = SKIER_STATE_SKIING;
        }

        skier_update(&skier);
        skier_draw(&skier);
    }

    return 0;
}
