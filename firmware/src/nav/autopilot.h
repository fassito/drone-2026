#pragma once

#include "nav/gps.h"
#include "nav/obstacle.h"
#include "nav/mission.h"
#include "nav/altitude_control.h"
#include "sensors/bmp388.h"
#include "sensors/mpu9250.h"

// ============================================================
// Piloto automático: genera setpoints de vuelo para llegar de
// forma autónoma a un waypoint, integrando la corrección de
// obstáculos, control de altitud y RTH seguro.
// Responsable: navigation-engineer.
// ============================================================

typedef struct {
    float max_tilt_rad;     // inclinación máxima de avance (pitch)
    float dist_gain;        // tilt por metro de distancia restante
    float dodge_tilt_rad;   // inclinación lateral al esquivar
    float target_alt_m;     // altitud objetivo (barómetro)
    float rth_alt_m;        // altitud de seguridad para el RTH
    altitude_controller_t alt;   // lazo de altitud
} autopilot_t;

void autopilot_init(autopilot_t *ap);

// Calcula roll_sp (rad), pitch_sp (rad), heading_error (rad) y throttle [0,1].
// heading_error se suma al yaw actual para el lazo de ángulo (desenvuelto).
// dt en segundos (1 kHz).
void autopilot_update(autopilot_t *ap,
                      const gps_state_t *gps,
                      const baro_state_t *baro,
                      const attitude_t *att,
                      const mission_state_t *mission,
                      const obstacle_correction_t *obs,
                      float dt,
                      float *roll_sp, float *pitch_sp,
                      float *heading_error, float *throttle);
