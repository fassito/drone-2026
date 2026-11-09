#pragma once

#include <stdbool.h>

// ============================================================
// Sensores de proximidad y evitación de choques.
// Sensores: 4x Sharp GP2Y0A21 (IR analógico, ADC1).
// Responsable: obstacle-avoidance-engineer. Skill: obstacle-avoidance.
// ============================================================

// STUB — pendiente de implementar.

#define PROX_DIRECTIONS 4   // frente, derecha, atrás, izquierda

typedef struct {
    // Distancias en cm; -1 = lectura inválida.
    float distance[PROX_DIRECTIONS];
    bool obstacle_near;   // true si alguna distancia < umbral de parada
} obstacle_state_t;

typedef struct {
    float speed_scale;    // 1.0 = sin restricción, 0.0 = parado
    int   avoid_dir;      // -1 izda, 0 ninguno, +1 dcha (desvío sugerido)
    bool  emergency_stop; // parada inmediata
} obstacle_correction_t;

void obstacle_init(void);
obstacle_state_t obstacle_read(void);
obstacle_correction_t obstacle_correction(const obstacle_state_t *state);
