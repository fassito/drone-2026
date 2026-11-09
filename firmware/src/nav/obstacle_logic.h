#pragma once

#include "nav/obstacle.h"

// ============================================================
// Lógica pura de obstáculos (sin hardware): conversión ADC→cm
// y corrección de evitación.
// ============================================================

#define PROX_WARN_CM 60.0f
#define PROX_STOP_CM 30.0f
#define PROX_CRIT_CM 15.0f

// Conversión ADC (10 bits) → distancia en cm para el Sharp GP2Y0A21.
float sharp_adc_to_cm(int adc);
