#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <Print.h>

// ============================================================
// Black box: registro circular en RAM de la telemetría de vuelo.
// Sin SD: guarda los últimos N segundos y se vuelca por CLI.
// ============================================================

typedef struct {
    uint32_t t_ms;
    float roll, pitch, yaw;          // rad
    float gyro_x, gyro_y, gyro_z;    // rad/s
    float throttle;                  // [0,1]
    float motor[4];                  // [0,1]
    int    mode;                     // flight_mode
    int    failsafe;                 // failsafe_mode
} blackbox_sample_t;

void blackbox_init(void);

// Graba una muestra (desde el lazo de control). Las muestras se limitan
// a BLACKBOX_MAX_SAMPLES de forma circular.
void blackbox_record(const blackbox_sample_t *s);

uint16_t blackbox_count(void);

// Vuelca las últimas muestras en CSV (desde la más antigua).
void blackbox_dump(Print &out);
