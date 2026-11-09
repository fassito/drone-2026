#include "nav/obstacle_logic.h"

// Curva no lineal del Sharp GP2Y0A21 (aproximación; calibrar LUT en hardware).
float sharp_adc_to_cm(int adc) {
    if (adc <= 20) return 80.0f;   // sin obstáculo detectable
    float cm = 4800.0f / (float)(adc - 20);
    if (cm < 10.0f) cm = 10.0f;
    if (cm > 80.0f) cm = 80.0f;
    return cm;
}

obstacle_correction_t obstacle_correction(const obstacle_state_t *s) {
    obstacle_correction_t c;
    c.speed_scale = 1.0f;
    c.avoid_dir = 0;
    c.emergency_stop = false;

    const float front = s->distance[0];
    const float right = s->distance[1];
    const float left = s->distance[3];

    // Parada de emergencia si algo está muy cerca en cualquier dirección.
    for (int i = 0; i < PROX_DIRECTIONS; i++) {
        if (s->distance[i] < PROX_CRIT_CM) {
            c.emergency_stop = true;
        }
    }
    if (c.emergency_stop) {
        c.speed_scale = 0.0f;
        return c;
    }

    // Frenar ante obstáculo frontal y sugerir desvío hacia el lado libre.
    if (front < PROX_STOP_CM) {
        c.speed_scale = 0.0f;
        c.avoid_dir = (right >= left) ? 1 : -1;
    } else if (front < PROX_WARN_CM) {
        c.speed_scale = 0.4f;
    }

    return c;
}
