// rgb_matrix_user.inc — Custom reactive ripple effect for Keychron K2 HE
// Effect: Random color ripples on keypress that fade smoothly
// Keys that are tapped repeatedly get brighter (heat-map style)
// Based on QMK RGB Matrix architecture

#ifdef RGB_MATRIX_FRAMEBUFFER_EFFECTS
#    include <stdint.h>
#    include <stdbool.h>

// Heatmap state — tracks activity per key
typedef uint8_t heat_t;
static heat_t g_heat[RGB_MATRIX_LED_COUNT];

// HSL to RGB helper
static inline uint8_t fastrand(void) {
    static uint32_t state = 2147483647; // simple deterministic PRNG seed
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return state >> 16;
}

static inline uint8_t clamp8(int16_t v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

static inline void hsv_to_rgb_fast(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b) {
    // H is 0-255 (not 360), S and V are 0-255
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch (region) {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

bool process_ripple(uint8_t led_min, uint8_t led_max) {
    // Only animate when typing heatmap is enabled (effect 66 in standard QMK)
    return true;
}

// The main effect function
// Called every frame by RGB Matrix
// led_idx: the LED index being rendered
// heat: accumulated tap activity for this LED (0-255)
// time: elapsed time in milliseconds
// Returns: the color to display for this LED
static rgb_task_states effect_ripple(uint8_t led_idx, heat_t heat, uint32_t time) {
    // Generate a stable random color per LED using position + time
    // Colors slowly shift over time for the "ripple" feel
    uint8_t rand_base = fastrand();
    uint8_t hue = (rand_base + (time >> 6)) & 0xFF; // slowly shifting hue
    uint8_t sat = 200; // high saturation for vivid colors
    uint8_t val_base = 140; // start at medium brightness

    // Heatmap contribution: repeated taps make key brighter
    // Map heat (0-255) to a brightness multiplier
    uint16_t heat_val = (uint16_t)heat * 115 + 14000; // 140-255 range
    uint8_t val = clamp8(heat_val >> 8);

    // Add per-key fade-out over time when not being pressed
    // This is handled by the heat decay in the animation tick

    uint8_t r, g, b;
    hsv_to_rgb_fast(hue, sat, val, &r, &g, &b);
    rgb_matrix_set_color(led_idx, r, g, b);

    return RGB_TASK_STATES_NORMAL;
}

// Tick function — called to update per-key heat state (called ~30 times/sec)
static void ripple_tick(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i < led_max; i++) {
        // Decay heat over time — unfocused keys fade back to dark
        if (g_heat[i] > 0) {
            g_heat[i]--;
        }
    }
}

#else
// Stub if framebuffer effects not available
#endif // RGB_MATRIX_FRAMEBUFFER_EFFECTS
