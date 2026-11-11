#include "telemetry/oled_display.h"
#include "telemetry/telemetry_data.h"
#include "sensors/mpu9250.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_SSD1306.h>

// SSD1306 0.96" 128x64. Para SH1106 1.3" usar la librería SH110X
// (ver skill oled-display/references/ssd1306-vs-sh1106.md).
static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static oled_view_t view = OLED_VIEW_BARO;
static bool ok = false;

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308232f
#endif

void oled_init(void) {
    ok = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
    if (ok) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.display();
    }
}

void oled_set_view(oled_view_t v) {
    view = v;
}

oled_view_t oled_get_view(void) {
    return view;
}

void oled_render(oled_view_t v) {
    if (!ok) return;

    display.clearDisplay();
    display.setCursor(0, 0);
    const drone_telemetry_t &t = g_telemetry;

    switch (v) {
        case OLED_VIEW_ATTITUDE:
            display.println("ATITUD (grados)");
            display.print("R:"); display.print(t.attitude.roll * RAD_TO_DEG, 1);
            display.print(" P:"); display.println(t.attitude.pitch * RAD_TO_DEG, 1);
            display.print("Y:"); display.print(t.attitude.yaw * RAD_TO_DEG, 1);
            display.print("  z:"); display.println(t.attitude.gyro_z, 1);
            display.print("mag:");
            display.println(mpu9250_mag_valid() ? "OK" : "NO");
            break;
        case OLED_VIEW_BARO:
            display.println("BAROMETRO");
            display.print("t:"); display.print(t.baro.temperature_c, 1);
            display.println(" C");
            display.print("p:"); display.print(t.baro.pressure_hpa, 1);
            display.println(" hPa");
            display.print("alt:"); display.print(t.baro.altitude_m, 1);
            display.println(" m");
            display.print("val:"); display.println(t.baro.valid ? "SI" : "NO");
            break;
        case OLED_VIEW_GPS:
            display.println("GPS");
            display.print("fix:"); display.println(t.gps.fix ? "SI" : "NO");
            display.print("sat:"); display.println(t.gps.sats);
            display.print("alt:"); display.println(t.gps.alt, 0);
            break;
        case OLED_VIEW_OBSTACLE:
            display.println("OBSTACULOS");
            display.print("F:"); display.print(t.obstacle.distance[0], 0);
            display.print(" D:"); display.println(t.obstacle.distance[1], 0);
            display.print("B:"); display.print(t.obstacle.distance[2], 0);
            display.print(" I:"); display.println(t.obstacle.distance[3], 0);
            break;
        case OLED_VIEW_STATUS:
        default:
            display.println("DRONE-2026");
            display.print("modo:"); display.print(t.flight_mode);
            display.print(" arm:"); display.println(t.failsafe.mode == FS_ARMED ? "SI" : "NO");
            display.print("bat:"); display.print(t.power.bus_voltage_v, 1);
            display.print("V "); display.print(t.battery.soc_percent, 0);
            display.println("%");
            display.print("rest:"); display.print(t.battery.remaining_mah, 0);
            display.print("mAh loop:"); display.println(t.loop_hz);
            break;
    }
    display.display();
}
