#include "safety/battery.h"

// Curva de descarga de una LiPo (tensión de celda en reposo → % de carga).
// Puntos tomados de la curva típica de LiPo de alta tasa de descarga.
static const float kCurveV[] = {
    4.20f, 4.10f, 4.00f, 3.90f, 3.80f, 3.70f,
    3.60f, 3.50f, 3.40f, 3.30f, 3.20f
};
static const float kCurvePct[] = {
    100.0f, 90.0f, 80.0f, 65.0f, 50.0f, 35.0f,
    25.0f, 15.0f, 8.0f, 3.0f, 0.0f
};
#define CURVE_POINTS ((int)(sizeof(kCurveV) / sizeof(kCurveV[0])))

float battery_cell_voltage(float vbat, int cells)
{
    if (cells <= 0) {
        return 0.0f;
    }
    return vbat / (float)cells;
}

float battery_soc_percent(float cell_v)
{
    if (cell_v >= kCurveV[0]) {
        return 100.0f;
    }
    if (cell_v <= kCurveV[CURVE_POINTS - 1]) {
        return 0.0f;
    }
    for (int i = 0; i < CURVE_POINTS - 1; i++) {
        if (cell_v <= kCurveV[i] && cell_v >= kCurveV[i + 1]) {
            const float span = kCurveV[i] - kCurveV[i + 1];
            const float t = (cell_v - kCurveV[i + 1]) / span;
            return kCurvePct[i + 1] + t * (kCurvePct[i] - kCurvePct[i + 1]);
        }
    }
    return 0.0f;
}

float battery_remaining_mah(float cell_v, float capacity_mah)
{
    if (capacity_mah <= 0.0f) {
        return 0.0f;
    }
    return capacity_mah * battery_soc_percent(cell_v) / 100.0f;
}

battery_state_t battery_evaluate(float vbat, int cells, float capacity_mah,
                                 float low_v, float critical_v,
                                 float min_plausible_v)
{
    battery_state_t s;
    s.cell_voltage_v = battery_cell_voltage(vbat, cells);
    s.plausible = (vbat > min_plausible_v);

    if (!s.plausible) {
        // Sensor sin cablear o desconectado: no se puede opinar.
        s.soc_percent = 0.0f;
        s.remaining_mah = 0.0f;
        s.level = BATTERY_LEVEL_OK;
        return s;
    }

    s.soc_percent = battery_soc_percent(s.cell_voltage_v);
    s.remaining_mah = battery_remaining_mah(s.cell_voltage_v, capacity_mah);

    if (vbat <= critical_v) {
        s.level = BATTERY_LEVEL_CRITICAL;
    } else if (vbat <= low_v) {
        s.level = BATTERY_LEVEL_LOW;
    } else {
        s.level = BATTERY_LEVEL_OK;
    }
    return s;
}
