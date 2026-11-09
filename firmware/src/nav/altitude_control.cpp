#include "nav/altitude_control.h"

void altitude_control_init(altitude_controller_t *ac) {
    pid_init(&ac->pid, 0.5f, 0.1f, 0.05f, 0.001f);
    pid_set_limits(&ac->pid, -0.3f, 0.3f, 0.3f, 0.02f);
    ac->hover_throttle = 0.40f;
    ac->prev_alt = 0.0f;
    ac->vel_z = 0.0f;
}

void altitude_control_set_hover(altitude_controller_t *ac, float hover) {
    if (hover < 0.1f) hover = 0.1f;
    if (hover > 0.9f) hover = 0.9f;
    ac->hover_throttle = hover;
}

void altitude_control_update(altitude_controller_t *ac, float target_alt_m,
                             float altitude_m, float dt, float *throttle_out) {
    // Estimación de velocidad vertical (derivada filtrada).
    if (dt > 1e-6f) {
        const float raw = (altitude_m - ac->prev_alt) / dt;
        ac->vel_z = ac->vel_z + 0.2f * (raw - ac->vel_z);   // filtro simple
    }
    ac->prev_alt = altitude_m;

    const float out = pid_step(&ac->pid, target_alt_m, altitude_m);
    float thr = ac->hover_throttle + out;
    if (thr < 0.0f) thr = 0.0f;
    if (thr > 1.0f) thr = 1.0f;
    *throttle_out = thr;
}
