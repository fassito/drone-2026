#pragma once

#include <stdint.h>

// ============================================================
// Señal OneShot125 (125–250 µs) para los 4 ESC BLHeli 30A.
// Responsable: esc-engineer. Skill: esc-setup.
// ============================================================

// Inicializa los 4 canales de salida (LEDC). Arranca SIN señal.
void esc_init(void);

// Habilita la salida de señal (solo con el sistema armado).
void esc_arm(void);

// Corta toda salida (desarmado / failsafe).
void esc_disarm(void);

// Escribe el ancho de pulso de un motor (motor: 0..3, pulse_us: 125..250).
// No emite señal si el sistema no está armado.
void esc_write_us(uint8_t motor, uint16_t pulse_us);

// Escribe un valor normalizado [0,1] convertido a OneShot125.
void esc_write_normalized(uint8_t motor, float value);
