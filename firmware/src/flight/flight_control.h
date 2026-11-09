#pragma once

#include "flight/pid.h"
#include "sensors/mpu9250.h"

// ============================================================
// Control de vuelo: PID en cascada (ángulo externo + rate interno)
// y mezcla de motores quad X.
// Responsable: flight-control-engineer.
// ============================================================

typedef enum {
    FLIGHT_MODE_DISARMED = 0,
    FLIGHT_MODE_ANGLE,    // estabilización de ángulo
    FLIGHT_MODE_RATE,     // acro / rate
    FLIGHT_MODE_NAV,      // navegación autónoma (waypoint/RTH)
} flight_mode_t;

typedef struct {
    pid_state_t rate_roll, rate_pitch, rate_yaw;
    pid_state_t angle_roll, angle_pitch, angle_yaw;
} flight_control_t;

void flight_control_init(flight_control_t *fc);

// sp_roll/sp_pitch en rad (modo ANGLE) o rad/s (modo RATE); sp_yaw en rad/s.
// throttle en [0,1]. Devuelve los valores de motor [0,1] en out[4].
void flight_control_update(flight_control_t *fc, flight_mode_t mode,
                           float roll_sp, float pitch_sp, float yaw_sp,
                           float throttle, const attitude_t *att, float out[4]);
