#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// Señal OneShot125 (125–250 µs) para los 4 ESC BLHeli 30A.
// Responsable: esc-engineer. Skill: esc-setup.
// ============================================================

// Inicializa los 4 canales de salida (LEDC). Arranca emitiendo throttle
// mínimo (125 µs) para que los ESC BLHeli puedan armarse, pero sin permitir
// subir de ahí hasta esc_arm().
void esc_init(void);

// Habilita la salida de señal (solo con el sistema armado).
void esc_arm(void);

// Vuelve al mínimo y bloquea el mando (desarmado / failsafe).
// No deja de emitir señal: los ESC necesitan el pulso mínimo continuo.
void esc_disarm(void);

// Escribe el ancho de pulso de un motor (motor: 0..3, pulse_us: 125..250).
// No emite señal si el sistema no está armado.
void esc_write_us(uint8_t motor, uint16_t pulse_us);

// Escribe un valor normalizado [0,1] convertido a OneShot125.
void esc_write_normalized(uint8_t motor, float value);

// Estado de habilitación del mando.
bool esc_is_armed(void);

// ---- Modo banco (pruebas SIN HÉLICES) ----
// Gira UN motor a un valor fijo durante un tiempo limitado, sin armado y
// sin pasar por el lazo de control. Pensado para identificar motores y
// verificar la configuración de los ESC en banco.
#define ESC_BENCH_MAX_MS 5000

void esc_bench_start(uint8_t motor, float value, uint32_t duration_ms);
void esc_bench_stop(void);
bool esc_bench_active(void);
uint8_t esc_bench_motor(void);
float esc_bench_value(void);

// Expira el modo banco si se cumplió el tiempo. Llamar periódicamente.
void esc_bench_tick(uint32_t now_ms);
