#include "gba.h"
#include "tiles.h"
#include "slope.h"
#include "skier.h"

// ── Input ─────────────────────────────────────────────────────────────────────
#define REG_KEYINPUT (*(volatile u16*)0x04000130)
#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)
#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)
#define KEY_R      (1 << 8)
#define KEY_L      (1 << 9)

// Keys are active-low: bit=0 means pressed
#define key_held(k)    (!( REG_KEYINPUT & (k)))

static u16 _prev_keys = 0xFFFF;
#define key_pressed(k) ( (_prev_keys & (k)) && !(REG_KEYINPUT & (k)))

static void input_update(void) { _prev_keys = REG_KEYINPUT; }

// ── Hype meter (simple) ───────────────────────────────────────────────────────
// Range 0..256 (fixed-int). Drives slope speed.
static int hype = 64; // start at 25%

static void hype_add(int delta) {
    hype += delta;
    if (hype > 256) hype = 256;
    if (hype < 0)   hype = 0;
}

static fixed hype_to_speed(void) {
    // Map hype 0..256 → speed 0.5..4.0 px/frame
    // speed = 0.5 + (hype/256) * 3.5
    return INT_TO_FX(1)/2 + (INT_TO_FX(7) / 2) * hype / 256;
}

// ── Demo jump trigger ─────────────────────────────────────────────────────────
// In this prototype: press A to launch a jump (will be automatic in full game)
int main(void) {
    // Display: Mode 0, BG0+BG1+BG2+OBJ enabled, 1D OBJ mapping
    REG_DISPCNT = DISPCNT_MODE0 | DISPCNT_BG0_ON | DISPCNT_BG1_ON |
                  DISPCNT_BG2_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D;

    // Load tile graphics and palettes
    tiles_load_placeholders();

    // Build tile maps and configure BG registers
    slope_build_maps();

    // Init slope state
    Slope slope;
    slope_init(&slope);

    // Init skier
    skier_load_sprite();
    Skier skier;
    skier_init(&skier, 112); // ground at pixel row 112 (= tile row 14 × 8)

    // Main loop
    while (1) {
        vsync();
        input_update();

        // Update hype → slope speed
        slope_set_speed(&slope, hype_to_speed());
        slope_update(&slope);

        // Jump demo: press A to launch, L/R to drain/fill hype
        if (key_pressed(KEY_A) && skier.state == SKIER_STATE_SKIING) {
            int height  = 32 + (hype * 24 / 256); // 32..56px peak height
            int airtime = 50 + (hype * 30 / 256); // 50..80 frames
            skier_launch(&skier, height, airtime);
        }
        if (key_held(KEY_L)) hype_add(-2);
        if (key_held(KEY_R)) hype_add(+2);

        // Allow landing → skiing transition after 1 frame
        if (skier.state == SKIER_STATE_LANDING) {
            hype_add(+20); // good landing bonus (placeholder)
            skier.state = SKIER_STATE_SKIING;
        }

        skier_update(&skier);
        skier_draw(&skier);
    }

    return 0;
}
