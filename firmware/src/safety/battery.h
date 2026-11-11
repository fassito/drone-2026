#pragma once

#include <stdbool.h>

// ============================================================
// Estado de energía de la batería a partir de la tensión del pack.
//
// El INA219 mide la tensión de la batería (y la corriente de la rama del
// BEC). Con la tensión se estima el estado de carga de la LiPo usando su
// curva de descarga; es la métrica que alimenta el failsafe y el aviso de
// "aterriza ya".
//
// Nota: la curva es aproximada. La tensión de una LiPo cae con la carga
// (efecto de la resistencia interna), así que el estado de carga es fiable
// en reposo o con poco gas, no a plena potencia.
// ============================================================

typedef enum {
    BATTERY_LEVEL_OK = 0,
    BATTERY_LEVEL_LOW,        // aterrizar
    BATTERY_LEVEL_CRITICAL    // corte de motores
} battery_level_t;

typedef struct {
    float cell_voltage_v;   // tensión media por celda
    float soc_percent;      // estado de carga estimado 0..100
    float remaining_mah;    // capacidad restante estimada
    battery_level_t level;
    bool  plausible;        // false si la lectura no es creíble
} battery_state_t;

// Tensión media por celda (vbat / celdas).
float battery_cell_voltage(float vbat, int cells);

// Estado de carga 0..100 % a partir de la tensión de una celda (curva LiPo).
float battery_soc_percent(float cell_v);

// Capacidad restante estimada (mAh) para un pack de capacidad nominal dada.
float battery_remaining_mah(float cell_v, float capacity_mah);

// Evalúa el pack completo.
//   low_v, critical_v: umbrales del pack (no por celda).
//   min_plausible_v:   por debajo, la lectura se considera no creíble.
battery_state_t battery_evaluate(float vbat, int cells, float capacity_mah,
                                 float low_v, float critical_v,
                                 float min_plausible_v);
