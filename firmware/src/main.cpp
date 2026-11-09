#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "safety/failsafe.h"
#include "safety/power_monitor.h"
#include "safety/indicators.h"
#include "safety/blackbox.h"
#include "sensors/mpu9250.h"
#include "sensors/bmp388.h"
#include "radio/ibus.h"
#include "actuators/esc.h"
#include "flight/flight_control.h"
#include "nav/gps.h"
#include "nav/obstacle.h"
#include "nav/mission.h"
#include "nav/geofence.h"
#include "telemetry/telemetry_data.h"
#include "telemetry/wifi_cli.h"
#include "telemetry/hc12_link.h"
#include "telemetry/oled_display.h"
#include "telemetry/cli.h"
#include "telemetry/ota.h"
#include "flight/flight_runtime.h"
#include "flight/pid_store.h"

drone_telemetry_t g_telemetry;

flight_control_t g_fc;
autotune_t g_autotune;
autopilot_t g_autopilot;
flight_mode_t g_flight_mode = FLIGHT_MODE_ANGLE;
geofence_t g_geofence;

static TaskHandle_t control_task_handle = nullptr;
static bool was_armed = false;
static bool was_failsafe = false;
static bool was_batt_low = false;

// Ajustes de mando (verificar según emisora FS-i6).
#define MAX_ANGLE_DEG      30.0f
#define MAX_RATE_RAD_S     3.5f
#define MAX_YAW_RATE_RAD_S 3.0f
#ifndef DEG_TO_RAD
#define DEG_TO_RAD         0.017453292519943295f
#endif

// Stream de salida para el HC-12 (mismo parser de la CLI que el WiFi).
class Hc12Stream : public Print {
public:
    size_t write(uint8_t c) override {
        hc12_write((const char *)&c, 1);
        return 1;
    }
    size_t write(const uint8_t *buffer, size_t size) override {
        hc12_write((const char *)buffer, size);
        return size;
    }
};
static Hc12Stream hc12_out;
static char hc12_line[96];
static uint8_t hc12_line_len = 0;

// Sondeo I2C para el diagnóstico de arranque.
static bool i2c_probe(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

// ------------------------------------------------------------
// Bucle de control: 1 kHz en el núcleo 0 (dedicado en exclusiva).
// ------------------------------------------------------------
static void control_task(void *arg) {
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1000 / LOOP_FREQ_HZ);
    uint32_t ticks = 0;
    uint32_t loops_in_second = 0;

    for (;;) {
        vTaskDelayUntil(&last_wake, period);
        ticks++;
        loops_in_second++;

        // 1) Sensores críticos.
#if ENABLE_MPU9250
        const attitude_t att = mpu9250_read();
#else
        attitude_t att;
        memset(&att, 0, sizeof(att));
        att.valid = false;
#endif
#if ENABLE_IBUS
        const ibus_state_t rx = ibus_read();
#else
        ibus_state_t rx;
        memset(&rx, 0, sizeof(rx));
        rx.link_ok = false;
#endif

        // 2) Mapeo de canales iBus.
        // ch0=roll, ch1=pitch, ch2=throttle, ch3=yaw, ch4=arm, ch5=mode
        const float throttle = (rx.channel[2] + 1.0f) * 0.5f;
        const bool throttle_min = rx.channel[2] < -0.9f;

        // 3) Failsafe: alimentar enlaces y energía.
        failsafe_set_rx_link(rx.link_ok);
        failsafe_set_imu_link(att.valid);
        failsafe_set_throttle_min(throttle_min);
        if (g_telemetry.power.valid) {
            failsafe_set_battery_low(g_telemetry.power.bus_voltage_v < BATTERY_CRITICAL_V);
            failsafe_set_overcurrent(g_telemetry.power.current_a > MAX_CURRENT_A);
        }
        failsafe_tick();

        // 4) Armado/desarmado desde el mando (CH5 alto = armado).
        const bool arm_sw = rx.channel[4] > 0.5f;
        const failsafe_mode_t fsmode = failsafe_get_state().mode;
        if (arm_sw && fsmode == FS_DISARMED) {
            if (failsafe_arm_request()) {
                bmp388_set_ground_reference();
            }
        } else if (!arm_sw && fsmode == FS_ARMED) {
            failsafe_disarm();
        }

        const bool armed = failsafe_get_state().mode == FS_ARMED;
        if (armed != was_armed) {
            was_armed = armed;
            wifi_set_enabled(!armed);   // WiFi solo en tierra
            indicators_event(armed ? IND_ARMED : IND_DISARMED);
        }

        // Indicaciones de failsafe y batería baja (transiciones).
        const bool now_failsafe = (failsafe_get_state().mode == FS_FAILSAFE);
        if (now_failsafe && !was_failsafe) {
            indicators_event(IND_FAILSAFE);
        }
        was_failsafe = now_failsafe;

        const bool now_batt_low = failsafe_get_state().battery_low;
        if (now_batt_low && !was_batt_low) {
            indicators_event(IND_BATTERY);
        }
        was_batt_low = now_batt_low;

        // 5) Modo de vuelo desde el mando (CH6) si no está fijado por CLI.
        //    El modo NAV solo se activa por CLI (no desde el mando).
        if (g_flight_mode != FLIGHT_MODE_NAV) {
            g_flight_mode = (rx.channel[5] > 0.5f) ? FLIGHT_MODE_RATE : FLIGHT_MODE_ANGLE;
        }

        // 6) Setpoints.
        float roll_sp = 0.0f, pitch_sp = 0.0f, yaw_sp = 0.0f;
        float throttle_cmd = throttle;
        flight_mode_t mode_eff = g_flight_mode;

        if (g_autotune.active) {
            // Autoajuste: excitación por relé en modo rate, un eje a la vez.
            mode_eff = FLIGHT_MODE_RATE;
            switch (g_autotune.axis) {
                case 0:
                    roll_sp = autotune_step(&g_autotune, &g_fc.rate_roll, att.gyro_x, att.roll, 0.001f);
                    break;
                case 1:
                    pitch_sp = autotune_step(&g_autotune, &g_fc.rate_pitch, att.gyro_y, att.pitch, 0.001f);
                    break;
                default:
                    yaw_sp = autotune_step(&g_autotune, &g_fc.rate_yaw, att.gyro_z, att.yaw, 0.001f);
                    break;
            }
        } else if (mode_eff == FLIGHT_MODE_NAV) {
            // Navegación autónoma (waypoint/RTH) + evitación de obstáculos.
            float heading_err = 0.0f;
            const obstacle_correction_t obs = obstacle_correction(&g_telemetry.obstacle);
            const mission_state_t m = mission_get();

            // Geofence: si se excede la distancia desde home → forzar RTH.
            geofence_state_t gs = geofence_check(&g_geofence, m.home_lat, m.home_lon,
                                                 &g_telemetry.gps, &g_telemetry.baro);
            if (gs.dist_exceeded && !m.is_rth) {
                mission_go_home();
            }
            // Avanzar al siguiente waypoint si se alcanzó el actual.
            mission_advance(&g_telemetry.gps);

            const mission_state_t m2 = mission_get();
            autopilot_update(&g_autopilot, &g_telemetry.gps, &g_telemetry.baro, &att,
                             &m2, &obs, 0.001f,
                             &roll_sp, &pitch_sp, &heading_err, &throttle_cmd);
            yaw_sp = att.yaw + heading_err;   // setpoint de ángulo desenvuelto
            mode_eff = FLIGHT_MODE_ANGLE;
        } else if (mode_eff == FLIGHT_MODE_ANGLE) {
            roll_sp = rx.channel[0] * MAX_ANGLE_DEG * DEG_TO_RAD;
            pitch_sp = rx.channel[1] * MAX_ANGLE_DEG * DEG_TO_RAD;
            yaw_sp = rx.channel[3] * MAX_YAW_RATE_RAD_S;
        } else {
            roll_sp = rx.channel[0] * MAX_RATE_RAD_S;
            pitch_sp = rx.channel[1] * MAX_RATE_RAD_S;
            yaw_sp = rx.channel[3] * MAX_YAW_RATE_RAD_S;
        }

        // 7) Control de vuelo y salida a motores.
        float motor[4] = {0, 0, 0, 0};
        if (armed) {
            flight_control_update(&g_fc, mode_eff, roll_sp, pitch_sp, yaw_sp,
                                  throttle_cmd, &att, motor);
            for (int i = 0; i < 4; i++) {
                esc_write_normalized(i, motor[i]);
            }
        } else {
            esc_disarm();
        }

        // 8) Telemetría agregada.
        g_telemetry.attitude = att;
        g_telemetry.failsafe = failsafe_get_state();
        g_telemetry.flight_mode = (int)g_flight_mode;
        g_telemetry.pid_rate_roll = g_fc.rate_roll.gains;
        g_telemetry.pid_rate_pitch = g_fc.rate_pitch.gains;
        g_telemetry.pid_rate_yaw = g_fc.rate_yaw.gains;
        for (int i = 0; i < 4; i++) g_telemetry.motor_out[i] = motor[i];

        // 8b) Black box a 100 Hz.
        if (ticks % (LOOP_FREQ_HZ / BLACKBOX_RATE_HZ) == 0) {
            blackbox_sample_t bb;
            bb.t_ms = millis();
            bb.roll = att.roll; bb.pitch = att.pitch; bb.yaw = att.yaw;
            bb.gyro_x = att.gyro_x; bb.gyro_y = att.gyro_y; bb.gyro_z = att.gyro_z;
            bb.throttle = throttle_cmd;
            for (int i = 0; i < 4; i++) bb.motor[i] = motor[i];
            bb.mode = (int)g_flight_mode;
            bb.failsafe = (int)failsafe_get_state().mode;
            blackbox_record(&bb);
        }

        // 9) Medición del lazo (1 Hz).
        if (ticks % LOOP_FREQ_HZ == 0) {
            g_telemetry.loop_hz = loops_in_second;
            loops_in_second = 0;
            Serial.printf("[ctrl] %luHz mode=%d arm=%d rx=%d imu=%d\n",
                          (unsigned long)g_telemetry.loop_hz,
                          (int)g_flight_mode, (int)armed,
                          (int)rx.link_ok, (int)att.valid);
        }
    }
}

// ------------------------------------------------------------
// Tarea de comunicaciones / sensores auxiliares (núcleo 1).
// ------------------------------------------------------------
static uint32_t last_oled = 0;
static uint32_t last_slow = 0;

static void slow_sensors_tick(void) {
    const uint32_t now = millis();
    if (now - last_slow < 200) return;
    last_slow = now;

#if ENABLE_GPS
    g_telemetry.gps = gps_read();
#endif
#if ENABLE_SHARP
    g_telemetry.obstacle = obstacle_read();
#endif
#if ENABLE_I2C
    g_telemetry.baro = bmp388_read();
    g_telemetry.power = power_monitor_read();
#endif

    // Guardar home al primer fix GPS (para RTH).
#if ENABLE_GPS
    mission_state_t m = mission_get();
    if (!m.home_set && g_telemetry.gps.fix) {
        mission_set_home(g_telemetry.gps.lat, g_telemetry.gps.lon);
    }
#endif
}

static void hc12_tick(void) {
#if ENABLE_HC12
    while (hc12_available()) {
        const char c = (char)hc12_read_byte();
        if (c == '\n' || c == '\r') {
            if (hc12_line_len > 0) {
                hc12_line[hc12_line_len] = '\0';
                cli_process_line(hc12_line, hc12_out);
                hc12_line_len = 0;
            }
        } else if (hc12_line_len < sizeof(hc12_line) - 1) {
            hc12_line[hc12_line_len++] = c;
        }
    }
#endif
}

static void oled_tick(void) {
#if ENABLE_I2C
    const uint32_t now = millis();
    if (now - last_oled < 200) return;
    last_oled = now;
    oled_render(oled_get_view());
#endif
}

void setup() {
    Serial.begin(MONITOR_SPEED);
    delay(200);
    Serial.println();
    Serial.println("Drone-2026 boot — " BOARD_NAME);

    pinMode(LED_BUILTIN, OUTPUT);

    failsafe_init();
    esc_init();
    indicators_init();
    blackbox_init();
    geofence_init(&g_geofence);
    flight_control_init(&g_fc);
    autotune_init(&g_autotune);
    autopilot_init(&g_autopilot);
    pid_store_load(&g_fc);   // cargar ganancias guardadas (si existen)

#if ENABLE_MPU9250
    mpu9250_init();
    Serial.printf("[imu] WHO_AM_I=0x%02X (0x71=ok)\n", mpu9250_who_am_i());
#endif
#if ENABLE_IBUS
    ibus_init();
#endif
#if ENABLE_I2C
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    // Muchos módulos OLED de 4 pines no traen pull-ups: activamos los
    // internos del ESP32 (~45 kΩ) para que el bus no quede flotando.
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    Serial.print("[i2c] bmp init...\n"); Serial.flush();
    bmp388_init();
    Serial.print("[i2c] ina init...\n"); Serial.flush();
    power_monitor_init();
    Serial.print("[i2c] oled init...\n"); Serial.flush();
    oled_init();
    Serial.print("[i2c] oled ok\n"); Serial.flush();
    // Escaneo completo del bus I2C.
    Serial.print("[i2c] dispositivos:");
    int found = 0;
    for (uint8_t a = 1; a < 127; a++) {
        if (i2c_probe(a)) {
            Serial.printf(" 0x%02X", a);
            found++;
        }
    }
    Serial.printf("  (total=%d)\n", found);
#endif
#if ENABLE_GPS
    gps_init();
#endif
#if ENABLE_SHARP
    obstacle_init();
#endif
    mission_init();
#if ENABLE_HC12
    hc12_init();
#endif
    wifi_cli_init();
    ota_init();

    // Lazo de control en el núcleo 0 (exclusivo). loop() queda en el 1.
    xTaskCreatePinnedToCore(control_task, "control", 8192, nullptr,
                            CONTROL_TASK_PRIORITY, &control_task_handle,
                            CONTROL_TASK_CORE);
}

void loop() {
    slow_sensors_tick();
    wifi_cli_tick();
    hc12_tick();
    oled_tick();
    indicators_tick();
    ota_handle();

    static uint32_t last_blink = 0;
    const uint32_t now = millis();
    if (now - last_blink >= 500) {
        last_blink = now;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}
