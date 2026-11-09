#include "actuators/esc.h"
#include "config.h"

#include <Arduino.h>

// ============================================================
// OneShot125 con LEDC.
//
// Timer LEDC a 80 MHz, resolución 15 bits → periodo 409.6 µs (~2.4 kHz)
// y paso de 12.5 ns. Un pulso de 125–250 µs equivale a 10000–20000 ticks
// (1 µs = 80 ticks). El periodo (409 µs) es mayor que el pulso máximo
// (250 µs), cumpliendo la temporización OneShot125.
// ============================================================

#define ONESHOT_TICKS_PER_US 80
#define ONESHOT_RESOLUTION 15
#define ONESHOT_FREQ_HZ (80000000 / (1 << ONESHOT_RESOLUTION))   // ~2441 Hz

static const uint8_t motor_pins[4] = {
    MOTOR_PIN_1, MOTOR_PIN_2, MOTOR_PIN_3, MOTOR_PIN_4
};
static const uint8_t motor_ch[4] = {0, 1, 2, 3};

static bool armed = false;

void esc_init(void) {
    for (int i = 0; i < 4; i++) {
        ledcSetup(motor_ch[i], ONESHOT_FREQ_HZ, ONESHOT_RESOLUTION);
        ledcAttachPin(motor_pins[i], motor_ch[i]);
        ledcWrite(motor_ch[i], 0);   // sin señal al arrancar
    }
    armed = false;
}

void esc_arm(void) {
    armed = true;
}

void esc_disarm(void) {
    armed = false;
    for (int i = 0; i < 4; i++) {
        ledcWrite(motor_ch[i], 0);
    }
}

void esc_write_us(uint8_t motor, uint16_t pulse_us) {
    if (motor >= 4) return;
    if (pulse_us < ONESHOT_MIN_US) pulse_us = ONESHOT_MIN_US;
    if (pulse_us > ONESHOT_MAX_US) pulse_us = ONESHOT_MAX_US;
    if (!armed) return;

    const uint32_t ticks = (uint32_t)pulse_us * ONESHOT_TICKS_PER_US;
    ledcWrite(motor_ch[motor], ticks);
}

void esc_write_normalized(uint8_t motor, float value) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    const uint16_t us = (uint16_t)(ONESHOT_MIN_US +
                        value * (ONESHOT_MAX_US - ONESHOT_MIN_US));
    esc_write_us(motor, us);
}
