#pragma once

#include "flight/flight_control.h"
#include "flight/autotune.h"
#include "nav/autopilot.h"

// ============================================================
// Estado de ejecución global del vuelo, compartido entre el
// lazo de control (core 0) y la CLI/telemetría (core 1).
// ============================================================

extern flight_control_t g_fc;
extern autotune_t g_autotune;
extern autopilot_t g_autopilot;
extern flight_mode_t g_flight_mode;
