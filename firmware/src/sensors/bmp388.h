#pragma once

#include <stdbool.h>

// ============================================================
// Barómetro BMP388 (I2C): presión, temperatura y altitud.
// La altitud barométrica es más precisa que la del GPS para el
// mantenimiento fino de altura (loopeo de altitud).
// Responsable: sensor-engineer / flight-control-engineer.
// ============================================================

// STUB — pendiente de implementar.

typedef struct {
    float pressure_hpa;   // presión
    float temperature_c;  // temperatura
    float altitude_m;     // altitud barométrica (referenciada al despegue)
    bool  valid;          // false si no hay lectura reciente
} baro_state_t;

void bmp388_init(void);

// Calibra la altitud de referencia (llamar al armar/despegar).
void bmp388_set_ground_reference(void);

baro_state_t bmp388_read(void);
