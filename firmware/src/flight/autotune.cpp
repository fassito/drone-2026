#include "flight/autotune.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define AUTOTUNE_STEPS 5000        // 5 s a 1 kHz
#define AUTOTUNE_KU_MIN 0.02f
#define AUTOTUNE_KU_MAX 2.0f

void autotune_init(autotune_t *at) {
    memset(at, 0, sizeof(*at));
    at->relay_amp = 1.0f;      // ±1 rad/s
    at->att_limit = 0.7f;      // ~40°
    at->kp_start = 0.1f;
}

void autotune_start(autotune_t *at, pid_state_t *rate_axis, uint8_t axis) {
    at->active = true;
    at->done = false;
    at->failed = false;
    at->axis = axis;
    at->step = 0;
    at->half_periods = 0;
    at->last_sign_step = 0;
    at->period_ms = 0.0f;
    at->rate_peak = -1e9f;
    at->rate_valley = 1e9f;
    at->last_rate_pos = true;
    at->backup = rate_axis->gains;

    // Solo P para excitar la oscilación límite.
    rate_axis->gains.kp = at->kp_start;
    rate_axis->gains.ki = 0.0f;
    rate_axis->gains.kd = 0.0f;
    pid_reset(rate_axis);

    snprintf(at->message, sizeof(at->message), "eje %u iniciado", (unsigned)axis);
}

float autotune_step(autotune_t *at, pid_state_t *rate_axis,
                    float rate_meas, float attitude, float dt) {
    if (!at->active) return 0.0f;

    // Divergencia: restaurar ganancias y abortar.
    if (fabsf(attitude) > at->att_limit) {
        at->active = false;
        at->failed = true;
        rate_axis->gains = at->backup;
        pid_reset(rate_axis);
        snprintf(at->message, sizeof(at->message), "abortado: actitud");
        return 0.0f;
    }

    // Seguimiento de picos y valles de la velocidad angular.
    if (rate_meas > at->rate_peak) at->rate_peak = rate_meas;
    if (rate_meas < at->rate_valley) at->rate_valley = rate_meas;

    // Conteo de semiciclos por cambio de signo → periodo último.
    const bool pos = (rate_meas >= 0.0f);
    if (pos != at->last_rate_pos) {
        if (at->last_sign_step != 0) {
            const float half_period_s = (float)(at->step - at->last_sign_step) * dt;
            at->period_ms = half_period_s * 2000.0f;   // medio → periodo completo
            at->half_periods++;
        }
        at->last_sign_step = at->step;
        at->last_rate_pos = pos;
    }

    // Relé: empuja en sentido contrario al signo de la medición.
    const float relay = pos ? -at->relay_amp : at->relay_amp;

    at->step++;
    if (at->step < AUTOTUNE_STEPS) {
        return relay;
    }

    // Finalizar y calcular Ziegler-Nichols.
    at->active = false;
    const float a = (at->rate_peak - at->rate_valley) / 2.0f;
    const float tu = at->period_ms / 1000.0f;

    if (a < 0.01f || tu < 0.005f || at->half_periods < 2) {
        at->failed = true;
        rate_axis->gains = at->backup;
        pid_reset(rate_axis);
        snprintf(at->message, sizeof(at->message), "sin oscilacion valida");
        return 0.0f;
    }

    float ku = 4.0f * at->relay_amp / (M_PI * a);
    if (ku < AUTOTUNE_KU_MIN) ku = AUTOTUNE_KU_MIN;
    if (ku > AUTOTUNE_KU_MAX) ku = AUTOTUNE_KU_MAX;

    autotune_zn_gains(ku, tu, &at->result);
    rate_axis->gains = at->result;
    pid_reset(rate_axis);
    at->done = true;
    snprintf(at->message, sizeof(at->message), "ok Ku=%.2f Tu=%.3fs", ku, tu);
    return 0.0f;
}

void autotune_zn_gains(float ku, float tu, pid_gains_t *out) {
    const float kp = 0.6f * ku;
    const float ti = 0.5f * tu;      // s
    const float td = 0.125f * tu;    // s
    out->kp = kp;
    out->ki = kp / ti;
    out->kd = kp * td;
}
