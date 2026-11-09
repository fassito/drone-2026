#include "nav/autopilot.h"
#include "nav/nav_math.h"

#include <math.h>

void autopilot_init(autopilot_t *ap) {
    ap->max_tilt_rad = 0.35f;      // ~20°
    ap->dist_gain = 0.05f;         // rad de pitch por metro
    ap->dodge_tilt_rad = 0.30f;    // ~17°
    ap->target_alt_m = 2.0f;
    ap->rth_alt_m = 5.0f;
    altitude_control_init(&ap->alt);
}

void autopilot_update(autopilot_t *ap,
                      const gps_state_t *gps,
                      const baro_state_t *baro,
                      const attitude_t *att,
                      const mission_state_t *mission,
                      const obstacle_correction_t *obs,
                      float dt,
                      float *roll_sp, float *pitch_sp,
                      float *heading_error, float *throttle) {
    *roll_sp = 0.0f;
    *pitch_sp = 0.0f;
    *heading_error = 0.0f;

    // Altitud de referencia para el lazo.
    float target_alt = ap->target_alt_m;
    if (mission->is_rth) {
        target_alt = ap->rth_alt_m;   // RTH: subir a altitud de seguridad
    }

    // Altitud (barómetro).
    if (baro && baro->valid) {
        altitude_control_update(&ap->alt, target_alt, baro->altitude_m, dt, throttle);
    } else {
        *throttle = ap->alt.hover_throttle;
    }

    // Parada de emergencia por obstáculo: mantener posición.
    if (obs && obs->emergency_stop) {
        *throttle = ap->alt.hover_throttle;
        return;
    }

    float bearing, distance;
    const bool nav_ok = mission_nav_info(mission, gps, &bearing, &distance);

    // RTH: si está por debajo de la altitud de seguridad, primero trepa.
    if (mission->is_rth && baro && baro->valid &&
        baro->altitude_m < ap->rth_alt_m - 0.5f) {
        // Fase de trepada: no avanzar, solo rumbo y altitud.
        *heading_error = nav_ok ? wrap_pi(bearing - att->yaw) : 0.0f;
        *pitch_sp = 0.0f;
        return;
    }

    if (!nav_ok) {
        // Sin misión o sin fix: mantener posición (hover).
        return;
    }

    // Rumbo hacia el objetivo (error desenvuelto).
    *heading_error = wrap_pi(bearing - att->yaw);

    // Avance proporcional a la distancia restante.
    float tilt = ap->dist_gain * distance;
    if (tilt > ap->max_tilt_rad) tilt = ap->max_tilt_rad;
    *pitch_sp = tilt;

    // Esquiva lateral si hay obstáculo frontal frenando.
    if (obs && obs->avoid_dir != 0 && obs->speed_scale < 0.5f) {
        *roll_sp = (float)obs->avoid_dir * ap->dodge_tilt_rad;
        *pitch_sp = 0.0f;
    }

    if (obs) {
        *pitch_sp *= obs->speed_scale;
    }
}
