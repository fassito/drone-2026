#include "flight/pid.h"

#include <math.h>

void pid_init(pid_state_t *pid, float kp, float ki, float kd, float dt) {
    pid->gains.kp = kp;
    pid->gains.ki = ki;
    pid->gains.kd = kd;
    pid->dt = dt;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_derivative = 0.0f;
    pid->out_min = -1.0f;
    pid->out_max = 1.0f;
    pid->integral_max = 1.0f;
    pid->deriv_tau = 0.01f;   // 10 ms
}

void pid_set_limits(pid_state_t *pid, float out_min, float out_max,
                    float integral_max, float deriv_tau) {
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integral_max = integral_max;
    pid->deriv_tau = deriv_tau;
}

float pid_step(pid_state_t *pid, float setpoint, float measurement) {
    const float error = setpoint - measurement;

    // Integral con anti-windup: se limita al rango permitido.
    pid->integral += error * pid->dt;
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < -pid->integral_max) {
        pid->integral = -pid->integral_max;
    }

    // Derivada con filtro paso bajo para no amplificar ruido.
    float derivative = 0.0f;
    if (pid->dt > 1e-6f) {
        const float raw = (error - pid->prev_error) / pid->dt;
        const float alpha = pid->dt / (pid->dt + pid->deriv_tau);
        derivative = pid->prev_derivative + alpha * (raw - pid->prev_derivative);
        pid->prev_derivative = derivative;
    }
    pid->prev_error = error;

    float out = pid->gains.kp * error
              + pid->gains.ki * pid->integral
              + pid->gains.kd * derivative;

    if (out > pid->out_max) {
        out = pid->out_max;
    } else if (out < pid->out_min) {
        out = pid->out_min;
    }
    return out;
}

void pid_reset(pid_state_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_derivative = 0.0f;
}
