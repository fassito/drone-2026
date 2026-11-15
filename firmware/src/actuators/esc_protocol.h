#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Protocolo OneShot125 (lógica pura, sin hardware).
//
// Aislado del driver LEDC para poder probarlo en el host. Lo usan tanto
// esc.cpp (salida real) como el CLI de pruebas.
// ============================================================

#define ESC_MIN_US 125
#define ESC_MAX_US 250

// Convierte un mando normalizado [0,1] a ancho de pulso OneShot125.
// Fuera de rango se recorta: 0 → 125 µs, 1 → 250 µs.
uint16_t esc_normalized_to_us(float value);

// Limita la variación de pulso por ciclo (rampa de aceleración).
// Devuelve el pulso a aplicar: avanza hacia target como mucho max_step_us.
uint16_t esc_ramp_us(uint16_t current_us, uint16_t target_us,
                     uint16_t max_step_us);

// true si el pulso está dentro del rango válido de OneShot125.
bool esc_pulse_in_range(uint16_t pulse_us);
