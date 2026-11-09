#include "nav/obstacle.h"
#include "nav/obstacle_logic.h"
#include "config.h"

#include <Arduino.h>

// ============================================================
// Sensores Sharp GP2Y0A21 (IR analógico, ADC1).
// Índices: 0=frente, 1=derecha, 2=atrás, 3=izquierda.
// La lógica pura (conversión y corrección) está en obstacle_logic.*.
// ============================================================

static const uint8_t prox_pins[PROX_DIRECTIONS] = {
    PROX_PIN_FRONT, PROX_PIN_RIGHT, PROX_PIN_BACK, PROX_PIN_LEFT
};

void obstacle_init(void) {
    analogReadResolution(10);   // 0..1023 (coincide con la curva documentada)
    for (int i = 0; i < PROX_DIRECTIONS; i++) {
        pinMode(prox_pins[i], INPUT);
    }
}

obstacle_state_t obstacle_read(void) {
    obstacle_state_t s;
    s.obstacle_near = false;
    for (int i = 0; i < PROX_DIRECTIONS; i++) {
        const int adc = analogRead(prox_pins[i]);
        s.distance[i] = sharp_adc_to_cm(adc);
        if (s.distance[i] < PROX_STOP_CM) {
            s.obstacle_near = true;
        }
    }
    return s;
}
