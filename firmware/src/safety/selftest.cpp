#include "safety/selftest.h"

#include "config.h"
#include "telemetry/telemetry_data.h"
#include "sensors/mpu9250.h"
#include "sensors/bmp388.h"
#include "safety/power_monitor.h"
#include "radio/ibus.h"
#include "nav/obstacle.h"
#include "nav/gps.h"

#include <Wire.h>
#include <stdio.h>

static int pass = 0;
static int fail = 0;

static void report(Print &out, bool ok, const char *name, const char *detail) {
    if (ok) { pass++; out.print("PASS "); }
    else    { fail++; out.print("FAIL "); }
    out.print(name);
    out.print(" — ");
    out.println(detail);
}

static bool i2c_has(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

void selftest_run(Print &out) {
    pass = 0;
    fail = 0;
    char buf[48];

    out.println("== Autotest Drone-2026 ==");

#if ENABLE_I2C
    report(out, i2c_has(OLED_I2C_ADDR), "OLED", "I2C 0x3C presente");
    report(out, i2c_has(INA219_I2C_ADDR), "INA219", "I2C 0x40 presente");
    report(out, i2c_has(BMP388_I2C_ADDR), "BMP388", "I2C 0x76/0x77 presente");
#else
    out.println("SKIP I2C (deshabilitado: ENABLE_I2C=0)");
#endif

#if ENABLE_MPU9250
    const uint8_t who = mpu9250_who_am_i();
    snprintf(buf, sizeof(buf), "WHO_AM_I=0x%02X", who);
    report(out, who == 0x71, "MPU9250", buf);
    report(out, g_telemetry.attitude.valid, "IMU actitud", "datos válidos");
#else
    out.println("SKIP MPU-9250 (deshabilitado: ENABLE_MPU9250=0)");
#endif

#if ENABLE_I2C
    baro_state_t b = bmp388_read();
    snprintf(buf, sizeof(buf), "alt=%.1fm press=%.1fhPa", b.altitude_m, b.pressure_hpa);
    report(out, b.valid, "BMP388 lectura", buf);

    power_state_t p = power_monitor_read();
    snprintf(buf, sizeof(buf), "V=%.2fV I=%.2fA", p.bus_voltage_v, p.current_a);
    report(out, p.valid && p.bus_voltage_v > 5.0f, "INA219 lectura", buf);
#endif

#if ENABLE_IBUS
    ibus_state_t rx = ibus_read();
    report(out, rx.link_ok, "iBus FS-IA6B", "enlace activo");
#else
    out.println("SKIP iBus (deshabilitado: ENABLE_IBUS=0)");
#endif

#if ENABLE_GPS
    gps_state_t g = gps_read();
    snprintf(buf, sizeof(buf), "fix=%d sats=%u", g.fix, g.sats);
    report(out, g.fix, "GPS", buf);
#else
    out.println("SKIP GPS (deshabilitado: ENABLE_GPS=0)");
#endif

#if ENABLE_SHARP
    obstacle_state_t o = obstacle_read();
    bool any_ok = false;
    for (int i = 0; i < PROX_DIRECTIONS; i++) {
        if (o.distance[i] >= 10.0f && o.distance[i] <= 80.0f) any_ok = true;
    }
    snprintf(buf, sizeof(buf), "F=%.0f D=%.0f B=%.0f I=%.0f",
             o.distance[0], o.distance[1], o.distance[2], o.distance[3]);
    report(out, any_ok, "Sharp GP2Y0A21", buf);
#else
    out.println("SKIP Sharp (deshabilitado: ENABLE_SHARP=0)");
#endif

    out.print("== Resultado: ");
    out.print(pass);
    out.print(" PASS, ");
    out.print(fail);
    out.println(" FAIL ==");
}
