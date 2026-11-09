#pragma once

#include "sensors/mpu9250.h"
#include "nav/gps.h"
#include "nav/obstacle.h"
#include "sensors/bmp388.h"
#include "safety/power_monitor.h"
#include "safety/failsafe.h"
#include "flight/pid.h"

// ============================================================
// Estado agregado de telemetría, compartido entre el lazo de
// control (core 0) y las tareas de comunicación/UI (core 1).
// Acceso: el lazo escribe; el resto solo lee.
// ============================================================

typedef struct {
    attitude_t attitude;
    gps_state_t gps;
    obstacle_state_t obstacle;
    baro_state_t baro;
    power_state_t power;
    failsafe_state_t failsafe;
    float motor_out[4];
    int flight_mode;       // 0=desarmado, 1=angle, 2=rate
    pid_gains_t pid_rate_roll;
    pid_gains_t pid_rate_pitch;
    pid_gains_t pid_rate_yaw;
    uint32_t loop_hz;
} drone_telemetry_t;

extern drone_telemetry_t g_telemetry;
