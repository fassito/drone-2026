#include "actuators/esc.h"
#include "actuators/esc_protocol.h"
#include "config.h"

#include <Arduino.h>

// ============================================================
// OneShot125 con LEDC.
//
// Timer LEDC a 80 MHz, resolución 15 bits → periodo 409.6 µs (~2.4 kHz)
// y paso de 12.5 ns. Un pulso de 125–250 µs equivale a 10000–20000 ticks
// (1 µs = 80 ticks). El periodo (409 µs) es mayor que el pulso máximo
// (250 µs), cumpliendo la temporización OneShot125.
//
// Importante: los ESC BLHeli necesitan ver throttle mínimo de forma
// continua para armarse y mantenerse armados. Se emite siempre 125 µs;
// "desarmado" significa que no se permite subir de ese valor.
// ============================================================

#define ONESHOT_TICKS_PER_US 80
#define ONESHOT_RESOLUTION 15
#define ONESHOT_FREQ_HZ (80000000 / (1 << ONESHOT_RESOLUTION))   // ~2441 Hz

// Rampa: 1 µs por ciclo de control (1 kHz) → 125→250 µs en 125 ms.
#define ESC_RAMP_MAX_STEP_US 1

static const uint8_t motor_pins[4] = {
    MOTOR_PIN_1, MOTOR_PIN_2, MOTOR_PIN_3, MOTOR_PIN_4
};
static const uint8_t motor_ch[4] = {0, 1, 2, 3};

static uint16_t current_us[4] = {ESC_MIN_US, ESC_MIN_US, ESC_MIN_US, ESC_MIN_US};
static bool armed = false;

static bool bench_active = false;
static uint8_t bench_motor = 0;
static float bench_value = 0.0f;
static uint32_t bench_deadline_ms = 0;

static void apply_us(uint8_t motor, uint16_t pulse_us) {
    const uint32_t ticks = (uint32_t)pulse_us * ONESHOT_TICKS_PER_US;
    ledcWrite(motor_ch[motor], ticks);
}

void esc_init(void) {
    for (int i = 0; i < 4; i++) {
        ledcSetup(motor_ch[i], ONESHOT_FREQ_HZ, ONESHOT_RESOLUTION);
        ledcAttachPin(motor_pins[i], motor_ch[i]);
        current_us[i] = ESC_MIN_US;
        apply_us(i, ESC_MIN_US);   // throttle mínimo: el ESC puede armarse
    }
    armed = false;
}

void esc_arm(void) {
    armed = true;
}

void esc_disarm(void) {
    armed = false;
    bench_active = false;
    for (int i = 0; i < 4; i++) {
        // Volver al mínimo de forma suave: cortar en seco desde alto RPM
        // puede dañar el ESC. La rampa la aplica esc_write_normalized.
        current_us[i] = ESC_MIN_US;
        apply_us(i, ESC_MIN_US);
    }
}

void esc_write_us(uint8_t motor, uint16_t pulse_us) {
    if (motor >= 4) return;
    if (pulse_us < ESC_MIN_US) pulse_us = ESC_MIN_US;
    if (pulse_us > ESC_MAX_US) pulse_us = ESC_MAX_US;
    if (!armed && !bench_active) return;

    const uint16_t applied = esc_ramp_us(current_us[motor], pulse_us,
                                         ESC_RAMP_MAX_STEP_US);
    current_us[motor] = applied;
    apply_us(motor, applied);
}

void esc_write_normalized(uint8_t motor, float value) {
    esc_write_us(motor, esc_normalized_to_us(value));
}

bool esc_is_armed(void) {
    return armed;
}

// ------------------------------------------------------------ modo banco

void esc_bench_start(uint8_t motor, float value, uint32_t duration_ms) {
    if (motor >= 4) return;
    if (duration_ms == 0) duration_ms = 1000;
    if (duration_ms > ESC_BENCH_MAX_MS) duration_ms = ESC_BENCH_MAX_MS;
    bench_motor = motor;
    bench_value = (value < 0.0f) ? 0.0f : (value > 1.0f ? 1.0f : value);
    bench_deadline_ms = millis() + duration_ms;
    bench_active = true;
}

void esc_bench_stop(void) {
    bench_active = false;
    for (int i = 0; i < 4; i++) {
        current_us[i] = ESC_MIN_US;
        apply_us(i, ESC_MIN_US);
    }
}

bool esc_bench_active(void) {
    return bench_active;
}

uint8_t esc_bench_motor(void) {
    return bench_motor;
}

float esc_bench_value(void) {
    return bench_value;
}

void esc_bench_tick(uint32_t now_ms) {
    if (bench_active && (int32_t)(now_ms - bench_deadline_ms) >= 0) {
        esc_bench_stop();
    }
}
