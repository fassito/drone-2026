#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Máquina de estados de seguridad: armado y failsafe.
// Responsable: drone-safety-reviewer (audita) / flight-control-engineer (integra).
// ============================================================

typedef enum {
    FS_DISARMED = 0,   // desarmado: motores cortados
    FS_ARMED,          // armado: lazo de control activo
    FS_FAILSAFE        // evento de seguridad: corte de motores
} failsafe_mode_t;

typedef struct {
    failsafe_mode_t mode;
    bool rx_link;        // enlace iBus del receptor OK
    bool imu_link;       // actitud de la IMU (MPU-9250) OK
    bool throttle_min;   // throttle en el mínimo (exigido para armar)
    bool battery_low;    // tensión por debajo del umbral crítico
    bool overcurrent;    // corriente por encima del límite
} failsafe_state_t;

void failsafe_init(void);
void failsafe_tick(void);   // debe llamarse a 1 kHz desde el lazo de control

failsafe_state_t failsafe_get_state(void);

bool failsafe_arm_request(void);   // arma solo si se cumplen todas las condiciones
void failsafe_disarm(void);

void failsafe_set_rx_link(bool ok);
void failsafe_set_imu_link(bool ok);
void failsafe_set_throttle_min(bool is_min);
void failsafe_set_battery_low(bool low);
void failsafe_set_overcurrent(bool over);
