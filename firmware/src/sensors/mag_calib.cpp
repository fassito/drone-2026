#include "sensors/mag_calib.h"

void mag_hard_iron_offsets(const float mn[3], const float mx[3], float offsets[3]) {
    for (int i = 0; i < 3; i++) {
        offsets[i] = (mn[i] + mx[i]) * 0.5f;
    }
}

void mag_compute_scale(const float mn[3], const float mx[3], float scale[3]) {
    // Radio de cada eje (mitad del rango); normalizar al mayor radio.
    float radius[3];
    float max_r = 1e-6f;
    for (int i = 0; i < 3; i++) {
        radius[i] = (mx[i] - mn[i]) * 0.5f;
        if (radius[i] > max_r) max_r = radius[i];
    }
    for (int i = 0; i < 3; i++) {
        scale[i] = (radius[i] > 1e-6f) ? (max_r / radius[i]) : 1.0f;
    }
}

void mag_apply_cal(const float raw[3], const float offsets[3],
                   const float scale[3], float out[3]) {
    for (int i = 0; i < 3; i++) {
        out[i] = (raw[i] - offsets[i]) * scale[i];
    }
}
