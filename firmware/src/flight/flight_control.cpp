#include "flight/flight_control.h"
#include "flight/mixer.h"

#include <math.h>

// Ganancias por defecto (conservadoras; reajustar con pid-autotune).
// Lazo de rate (rad/s → salida de motor normalizada).
#define RATE_ROLL_KP  0.10f
#define RATE_ROLL_KI  0.30f
#define RATE_ROLL_KD  0.003f
#define RATE_PITCH_KP 0.10f
#define RATE_PITCH_KI 0.30f
#define RATE_PITCH_KD 0.003f
#define RATE_YAW_KP   0.25f
#define RATE_YAW_KI   0.05f
#define RATE_YAW_KD   0.0f

// Lazo de ángulo (rad → rad/s de rate).
#define ANGLE_ROLL_KP  8.0f
#define ANGLE_PITCH_KP 8.0f
#define ANGLE_YAW_KP   3.0f

// Máximos de rate solicitados por el lazo de ángulo (rad/s).
#define ANGLE_RATE_LIMIT 5.0f

void flight_control_init(flight_control_t *fc) {
    const float dt = 0.001f;   // 1 kHz

    pid_init(&fc->rate_roll, RATE_ROLL_KP, RATE_ROLL_KI, RATE_ROLL_KD, dt);
    pid_init(&fc->rate_pitch, RATE_PITCH_KP, RATE_PITCH_KI, RATE_PITCH_KD, dt);
    pid_init(&fc->rate_yaw, RATE_YAW_KP, RATE_YAW_KI, RATE_YAW_KD, dt);
    pid_init(&fc->angle_roll, ANGLE_ROLL_KP, 0.0f, 0.0f, dt);
    pid_init(&fc->angle_pitch, ANGLE_PITCH_KP, 0.0f, 0.0f, dt);
    pid_init(&fc->angle_yaw, ANGLE_YAW_KP, 0.0f, 0.0f, dt);

    pid_set_limits(&fc->rate_roll, -1.0f, 1.0f, 1.0f, 0.01f);
    pid_set_limits(&fc->rate_pitch, -1.0f, 1.0f, 1.0f, 0.01f);
    pid_set_limits(&fc->rate_yaw, -1.0f, 1.0f, 1.0f, 0.01f);
    pid_set_limits(&fc->angle_roll, -ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, 0.02f);
    pid_set_limits(&fc->angle_pitch, -ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, 0.02f);
    pid_set_limits(&fc->angle_yaw, -ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, ANGLE_RATE_LIMIT, 0.02f);
}

void flight_control_update(flight_control_t *fc, flight_mode_t mode,
                           float roll_sp, float pitch_sp, float yaw_sp,
                           float throttle, const attitude_t *att, float out[4]) {
    float rate_roll_sp, rate_pitch_sp, rate_yaw_sp;

    if (mode == FLIGHT_MODE_ANGLE) {
        // Lazo externo: el error de ángulo genera un setpoint de rate.
        rate_roll_sp  = pid_step(&fc->angle_roll, roll_sp, att->roll);
        rate_pitch_sp = pid_step(&fc->angle_pitch, pitch_sp, att->pitch);
        rate_yaw_sp   = pid_step(&fc->angle_yaw, yaw_sp, att->yaw);
    } else {
        // Modo rate: setpoints directos de velocidad angular.
        rate_roll_sp  = roll_sp;
        rate_pitch_sp = pitch_sp;
        rate_yaw_sp   = yaw_sp;
    }

    // Lazo interno de rate.
    const float roll_out  = pid_step(&fc->rate_roll, rate_roll_sp, att->gyro_x);
    const float pitch_out = pid_step(&fc->rate_pitch, rate_pitch_sp, att->gyro_y);
    const float yaw_out   = pid_step(&fc->rate_yaw, rate_yaw_sp, att->gyro_z);

    mixer_apply(roll_out, pitch_out, yaw_out, throttle, out);
}
