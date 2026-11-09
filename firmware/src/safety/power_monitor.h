#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Monitor de batería (INA219 por I2C): tensión y corriente.
// La lectura es I2C (lenta): se hace a baja frecuencia desde una
// tarea separada; el failsafe consume el último valor válido.
// Responsable: flight-control-engineer / drone-safety-reviewer.
// ============================================================

// STUB — pendiente de implementar.

typedef struct {
    float bus_voltage_v;   // tensión de batería
    float current_a;       // corriente consumida
    float power_w;         // potencia
    bool  valid;           // false si no hay lectura reciente
} power_state_t;

void power_monitor_init(void);
power_state_t power_monitor_read(void);

// Umbral de batería baja para el failsafe (configurable en config.h).
bool power_monitor_battery_low(const power_state_t *state);
