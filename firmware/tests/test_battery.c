#include "test_util.h"
#include "safety/battery.h"

static void test_cell_voltage(void)
{
    test_section("Batería: tensión por celda");
    CHECK_NEAR(battery_cell_voltage(16.8f, 4), 4.2f, 0.001f, "4S 16.8 V → 4.20 V/celda");
    CHECK_NEAR(battery_cell_voltage(12.6f, 3), 4.2f, 0.001f, "3S 12.6 V → 4.20 V/celda");
    CHECK(battery_cell_voltage(16.8f, 0) == 0.0f, "0 celdas → 0 (no divide por cero)");
}

static void test_soc_curve(void)
{
    test_section("Batería: curva de carga");
    CHECK_NEAR(battery_soc_percent(4.20f), 100.0f, 0.01f, "4.20 V → 100 %");
    CHECK_NEAR(battery_soc_percent(4.30f), 100.0f, 0.01f, "por encima → 100 % (limitado)");
    CHECK_NEAR(battery_soc_percent(3.80f), 50.0f, 0.01f, "3.80 V → 50 %");
    CHECK_NEAR(battery_soc_percent(3.30f), 3.0f, 0.01f, "3.30 V → 3 %");
    CHECK_NEAR(battery_soc_percent(3.00f), 0.0f, 0.01f, "por debajo → 0 % (limitado)");

    // monotonía: más tensión nunca da menos carga
    float prev = -1.0f;
    bool mono = true;
    for (float v = 3.2f; v <= 4.2f; v += 0.05f) {
        const float s = battery_soc_percent(v);
        if (s < prev) {
            mono = false;
        }
        prev = s;
    }
    CHECK(mono, "la curva es monótona creciente");
}

static void test_remaining(void)
{
    test_section("Batería: capacidad restante");
    CHECK_NEAR(battery_remaining_mah(3.80f, 1550.0f), 775.0f, 1.0f,
               "50 % de 1550 mAh → 775 mAh");
    CHECK_NEAR(battery_remaining_mah(4.20f, 1550.0f), 1550.0f, 1.0f,
               "100 % → capacidad completa");
    CHECK(battery_remaining_mah(4.0f, 0.0f) == 0.0f, "capacidad 0 → 0");
}

static void test_evaluate(void)
{
    test_section("Batería: evaluación del pack 4S");
    // 4S: low 14.0 V, critical 13.2 V, plausible > 6 V
    battery_state_t s = battery_evaluate(16.8f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(s.plausible, "16.8 V es creíble");
    CHECK(s.level == BATTERY_LEVEL_OK, "16.8 V → OK");
    CHECK_NEAR(s.cell_voltage_v, 4.2f, 0.001f, "16.8 V → 4.20 V/celda");
    CHECK_NEAR(s.soc_percent, 100.0f, 0.01f, "16.8 V → 100 %");

    s = battery_evaluate(14.8f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(s.level == BATTERY_LEVEL_OK, "14.8 V (nominal) → OK");

    s = battery_evaluate(13.9f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(s.level == BATTERY_LEVEL_LOW, "13.9 V → LOW (aterrizar)");
    CHECK(s.soc_percent > 0.0f && s.soc_percent < 20.0f, "13.9 V → carga baja");

    s = battery_evaluate(13.2f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(s.level == BATTERY_LEVEL_CRITICAL, "13.2 V → CRITICAL (corte, límite incluido)");

    s = battery_evaluate(12.5f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(s.level == BATTERY_LEVEL_CRITICAL, "12.5 V → CRITICAL");

    s = battery_evaluate(0.86f, 4, 1550.0f, 14.0f, 13.2f, 6.0f);
    CHECK(!s.plausible, "0.86 V no es creíble (sensor sin Vin)");
    CHECK(s.level == BATTERY_LEVEL_OK, "lectura no creíble → no dispara failsafe");
}

void test_battery(void)
{
    test_cell_voltage();
    test_soc_curve();
    test_remaining();
    test_evaluate();
}
