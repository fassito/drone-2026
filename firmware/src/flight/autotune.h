#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "flight/pid.h"

// ============================================================
// Autoajuste PID por realimentación de relé (Åström–Hägglund)
// + reglas de Ziegler-Nichols.
// Responsable: pid-tuning-engineer. Skill: pid-autotune.
// ============================================================

typedef struct {
    bool active;
    bool done;
    bool failed;
    uint8_t axis;           // 0=roll, 1=pitch, 2=yaw
    float relay_amp;        // amplitud del relé en rad/s
    float att_limit;        // límite de actitud para abortar (rad)
    float kp_start;         // Kp inicial (solo P) durante la excitación
    uint32_t step;
    uint32_t half_periods;
    uint32_t last_sign_step;
    float period_ms;        // periodo último de oscilación
    float rate_peak;
    float rate_valley;
    bool  last_rate_pos;
    pid_gains_t result;
    pid_gains_t backup;     // ganancias previas (restaurar si aborta)
    char message[48];
} autotune_t;

void autotune_init(autotune_t *at);

// Inicia el autoajuste sobre el lazo de rate del eje indicado.
// Guarda las ganancias previas y pone el lazo en modo P.
void autotune_start(autotune_t *at, pid_state_t *rate_axis, uint8_t axis);

// Avanza la máquina de estados (llamar a 1 kHz desde el lazo de control).
// rate_meas en rad/s, attitude en rad, dt en segundos.
// Devuelve el setpoint de rate a inyectar (0 cuando no aplica).
// Al terminar aplica las ganancias calculadas o restaura las previas.
float autotune_step(autotune_t *at, pid_state_t *rate_axis,
                    float rate_meas, float attitude, float dt);

// Reglas de Ziegler-Nichols a partir de ganancia crítica Ku y periodo Tu (s).
void autotune_zn_gains(float ku, float tu, pid_gains_t *out);
