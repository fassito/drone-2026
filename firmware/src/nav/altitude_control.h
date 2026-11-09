#pragma once

#include "flight/pid.h"

// ============================================================
// Control de altitud (PID sobre barómetro). Genera el throttle
// para mantener una altitud objetivo.
// ============================================================

typedef struct {
    pid_state_t pid;
    float hover_throttle;   // throttle base para hover
    float prev_alt;
    float vel_z;            // velocidad vertical estimada (m/s)
} altitude_controller_t;

void altitude_control_init(altitude_controller_t *ac);

// Ajusta el throttle base (se puede autoestimar en vuelo).
void altitude_control_set_hover(altitude_controller_t *ac, float hover);

// Devuelve el throttle [0,1] para seguir la altitud objetivo.
void altitude_control_update(altitude_controller_t *ac, float target_alt_m,
                             float altitude_m, float dt, float *throttle_out);
