#pragma once

// ============================================================
// PID en cascada: lazo externo de ángulo + lazo interno de rate.
// Responsables: flight-control-engineer (estructura) y
//               pid-tuning-engineer (autoajuste).
// ============================================================

// Parámetros por eje.
typedef struct {
    float kp;
    float ki;
    float kd;
} pid_gains_t;

// Estado de un PID (un eje, un lazo).
typedef struct {
    pid_gains_t gains;
    float integral;
    float prev_error;
    float prev_derivative;
    float dt;
    float out_min;      // límite inferior de salida
    float out_max;      // límite superior de salida
    float integral_max; // límite anti-windup del integrador
    float deriv_tau;    // constante de tiempo del filtro derivativo (s)
} pid_state_t;

void pid_init(pid_state_t *pid, float kp, float ki, float kd, float dt);
void pid_set_limits(pid_state_t *pid, float out_min, float out_max,
                    float integral_max, float deriv_tau);

// Devuelve la salida limitada al rango [out_min, out_max].
float pid_step(pid_state_t *pid, float setpoint, float measurement);

void pid_reset(pid_state_t *pid);
