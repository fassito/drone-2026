#pragma once

#include "flight/flight_control.h"

// ============================================================
// Persistencia de ganancias PID en NVS (Preferences).
// ============================================================

void pid_store_save(const flight_control_t *fc);
void pid_store_load(flight_control_t *fc);
