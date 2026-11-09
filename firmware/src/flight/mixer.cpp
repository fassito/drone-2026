#include "flight/mixer.h"

// ============================================================
// Mezcla de motores quad X.
//
// Orden de salida (según mixer.h):
//   out[0] = Motor 1 (FR, CW)   delante-derecha
//   out[1] = Motor 2 (RL, CW)   detrás-izquierda
//   out[2] = Motor 3 (FL, CCW)  delante-izquierda
//   out[3] = Motor 4 (RR, CCW)  detrás-derecha
//
// Convenciones:
//   roll  +1 = inclinar a la derecha
//   pitch +1 = inclinar hacia delante (nariz abajo)
//   yaw   +1 = girar a la derecha (horario visto desde arriba)
//   throttle  [0, +1]
//
// Los signos de yaw asumen la mezcla CW/CCW indicada. Verificar
// empíricamente en banco de pruebas (skill preflight-safety).
// ============================================================

static float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void mixer_apply(float roll, float pitch, float yaw, float throttle, float out[4]) {
    out[0] = throttle - roll - pitch - yaw;   // FR (CW)
    out[1] = throttle + roll + pitch - yaw;   // RL (CW)
    out[2] = throttle + roll - pitch + yaw;   // FL (CCW)
    out[3] = throttle - roll + pitch + yaw;   // RR (CCW)

    for (int i = 0; i < 4; i++) {
        out[i] = clamp01(out[i]);
    }
}
