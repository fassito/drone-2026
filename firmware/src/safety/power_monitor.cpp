#include "safety/power_monitor.h"
#include "config.h"

#include <Adafruit_INA219.h>

static Adafruit_INA219 ina(INA219_I2C_ADDR);
static bool ok = false;

void power_monitor_init(void) {
    ok = ina.begin();
    if (ok) {
        // Configuración por defecto: shunt 0.1 Ω, rango 32 V / 2 A.
        ina.setCalibration_32V_2A();
    }
}

power_state_t power_monitor_read(void) {
    power_state_t s;
    s.bus_voltage_v = 0.0f;
    s.current_a = 0.0f;
    s.power_w = 0.0f;
    s.valid = false;

    if (!ok) return s;

    s.bus_voltage_v = ina.getBusVoltage_V();
    s.current_a = ina.getCurrent_mA() / 1000.0f;
    s.power_w = ina.getPower_mW() / 1000.0f;
    s.valid = true;
    return s;
}

bool power_monitor_battery_low(const power_state_t *state) {
    return state->valid && state->bus_voltage_v < BATTERY_LOW_V;
}
