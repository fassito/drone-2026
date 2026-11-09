#pragma once

#include <Print.h>

// ============================================================
// Autotest en el dispositivo (con componentes activos).
// Ejecuta comprobaciones no destructivas de cada subsistema y
// reporta PASS/FAIL. No mueve motores.
// ============================================================

void selftest_run(Print &out);
