#include "telemetry/cli.h"
#include "telemetry/telemetry_data.h"
#include "telemetry/wifi_cli.h"
#include "telemetry/oled_display.h"

#include "safety/failsafe.h"
#include "safety/selftest.h"
#include "safety/blackbox.h"
#include "actuators/esc.h"
#include "nav/obstacle.h"
#include "nav/mission.h"
#include "nav/geofence.h"
#include "sensors/bmp388.h"
#include "sensors/mpu9250.h"
#include "flight/flight_runtime.h"
#include "flight/pid_store.h"

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308232f
#endif

static void print_help(Print &out) {
    out.println("help status attitude gps power obstacle baro pid");
    out.println("arm confirm | disarm");
    out.println("mode angle|rate|nav");
    out.println("mission set <lat> <lon> | mission stop | rth confirm");
    out.println("calibrate gyro|mag | geofence | blackbox");
    out.println("autotune start|stop|status");
    out.println("pid save | pid load");
    out.println("selftest | json");
    out.println("oled status|attitude|baro|gps|obstacle");
    out.println("wifi on|off | reboot");
}

static void cmd_arm(Print &out) {
    if (failsafe_arm_request()) {
        esc_arm();
        out.println("OK armado");
    } else {
        const failsafe_state_t s = failsafe_get_state();
        out.print("ERROR armado rechazado rx=");
        out.print(s.rx_link); out.print(" imu="); out.print(s.imu_link);
        out.print(" thrmin="); out.println(s.throttle_min);
    }
}

void cli_process_line(const char *line, Print &out) {
    // Recortar CR/LF.
    char buf[80];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    size_t n = strlen(buf);
    while (n > 0 && (buf[n - 1] == '\r' || buf[n - 1] == '\n' || buf[n - 1] == ' ')) {
        buf[--n] = '\0';
    }

    const char *cmd = buf;
    // separar primer token
    char *sp = strchr(buf, ' ');
    if (sp) { *sp = '\0'; }

    if (!cmd[0]) return;

    if (strcmp(cmd, "help") == 0) {
        print_help(out);
    } else if (strcmp(cmd, "status") == 0) {
        const failsafe_state_t s = g_telemetry.failsafe;
        out.print("modo="); out.print(g_telemetry.flight_mode);
        out.print(" armado="); out.print(s.mode == FS_ARMED ? 1 : 0);
        out.print(" rx="); out.print(s.rx_link);
        out.print(" imu="); out.print(s.imu_link);
        out.print(" loop="); out.println(g_telemetry.loop_hz);
    } else if (strcmp(cmd, "attitude") == 0) {
        const attitude_t &a = g_telemetry.attitude;
        out.print("roll="); out.print(a.roll * RAD_TO_DEG, 1);
        out.print(" pitch="); out.print(a.pitch * RAD_TO_DEG, 1);
        out.print(" yaw="); out.print(a.yaw * RAD_TO_DEG, 1);
        out.println();
        out.print("gyro_x="); out.print(a.gyro_x, 2);
        out.print(" gyro_y="); out.print(a.gyro_y, 2);
        out.print(" gyro_z="); out.print(a.gyro_z, 2);
        out.println();
        out.print("mag="); out.print(mpu9250_mag_present() ? 1 : 0);
        out.print(" cal="); out.print(mpu9250_mag_valid() ? 1 : 0);
        out.print(" valid="); out.println(a.valid ? 1 : 0);
    } else if (strcmp(cmd, "gps") == 0) {
        const gps_state_t &g = g_telemetry.gps;
        out.print("fix="); out.print(g.fix);
        out.print(" sats="); out.print(g.sats);
        out.print(" lat="); out.print(g.lat, 6);
        out.print(" lon="); out.print(g.lon, 6);
        out.print(" alt="); out.println(g.alt, 1);
    } else if (strcmp(cmd, "power") == 0) {
        const power_state_t &p = g_telemetry.power;
        out.print("v="); out.print(p.bus_voltage_v, 2);
        out.print(" i="); out.print(p.current_a, 2);
        out.print(" w="); out.print(p.power_w, 1);
        out.print(" low="); out.println(power_monitor_battery_low(&p) ? 1 : 0);
    } else if (strcmp(cmd, "obstacle") == 0) {
        obstacle_state_t s = obstacle_read();
        out.print("front="); out.print(s.distance[0], 0);
        out.print(" right="); out.print(s.distance[1], 0);
        out.print(" back="); out.print(s.distance[2], 0);
        out.print(" left="); out.println(s.distance[3], 0);
    } else if (strcmp(cmd, "baro") == 0) {
        baro_state_t b = bmp388_read();
        out.print("alt="); out.print(b.altitude_m, 1);
        out.print(" press="); out.print(b.pressure_hpa, 1);
        out.print(" temp="); out.println(b.temperature_c, 1);
    } else if (strcmp(cmd, "pid") == 0) {
        if (sp && strstr(sp + 1, "save")) {
            pid_store_save(&g_fc);
            out.println("OK pid guardado");
        } else if (sp && strstr(sp + 1, "load")) {
            pid_store_load(&g_fc);
            out.println("OK pid cargado");
        } else {
            out.print("rate_roll  "); out.print(g_telemetry.pid_rate_roll.kp); out.print(" ");
            out.print(g_telemetry.pid_rate_roll.ki); out.print(" "); out.println(g_telemetry.pid_rate_roll.kd);
            out.print("rate_pitch "); out.print(g_telemetry.pid_rate_pitch.kp); out.print(" ");
            out.print(g_telemetry.pid_rate_pitch.ki); out.print(" "); out.println(g_telemetry.pid_rate_pitch.kd);
            out.print("rate_yaw   "); out.print(g_telemetry.pid_rate_yaw.kp); out.print(" ");
            out.print(g_telemetry.pid_rate_yaw.ki); out.print(" "); out.println(g_telemetry.pid_rate_yaw.kd);
        }
    } else if (strcmp(cmd, "mode") == 0) {
        if (sp && strstr(sp + 1, "angle")) {
            g_flight_mode = FLIGHT_MODE_ANGLE;
            out.println("OK angle");
        } else if (sp && strstr(sp + 1, "rate")) {
            g_flight_mode = FLIGHT_MODE_RATE;
            out.println("OK rate");
        } else if (sp && strstr(sp + 1, "nav")) {
            g_flight_mode = FLIGHT_MODE_NAV;
            out.println("OK nav");
        } else {
            out.println("uso: mode angle|rate|nav");
        }
    } else if (strcmp(cmd, "mission") == 0) {
        if (sp && strstr(sp + 1, "stop")) {
            mission_stop();
            out.println("OK mision detenida");
        } else if (sp && strstr(sp + 1, "set")) {
            float lat = 0.0f, lon = 0.0f;
            if (sscanf(sp + 1, " set %f %f", &lat, &lon) == 2) {
                mission_set_target(lat, lon);
                out.println("OK destino fijado");
            } else {
                out.println("uso: mission set <lat> <lon>");
            }
        } else {
            mission_state_t m = mission_get();
            out.print("active="); out.print(m.active);
            out.print(" home="); out.println(m.home_set);
        }
    } else if (strcmp(cmd, "rth") == 0) {
        if (sp && strstr(sp + 1, "confirm")) {
            mission_go_home();
            out.println("OK RTH");
        } else {
            out.println("usa 'rth confirm'");
        }
    } else if (strcmp(cmd, "autotune") == 0) {
        if (sp && strstr(sp + 1, "start")) {
            autotune_start(&g_autotune, &g_fc.rate_roll, 0);
            out.println("OK autotune roll iniciado");
        } else if (sp && strstr(sp + 1, "stop")) {
            g_autotune.active = false;
            g_fc.rate_roll.gains = g_autotune.backup;
            pid_reset(&g_fc.rate_roll);
            out.println("OK autotune detenido");
        } else {
            out.print("active="); out.print(g_autotune.active);
            out.print(" done="); out.print(g_autotune.done);
            out.print(" failed="); out.print(g_autotune.failed);
            out.print(" msg="); out.println(g_autotune.message);
        }
    } else if (strcmp(cmd, "selftest") == 0) {
        selftest_run(out);
    } else if (strcmp(cmd, "json") == 0) {
        // Telemetría en una línea JSON (para graficar en PC).
        const drone_telemetry_t &t = g_telemetry;
        out.print("{\"t\":{\"roll\":");
        out.print(t.attitude.roll, 4); out.print(",\"pitch\":");
        out.print(t.attitude.pitch, 4); out.print(",\"yaw\":");
        out.print(t.attitude.yaw, 4); out.print(",\"mode\":");
        out.print(t.flight_mode); out.print(",\"failsafe\":");
        out.print((int)t.failsafe.mode); out.print(",\"gps_fix\":");
        out.print(t.gps.fix ? 1 : 0); out.print(",\"sats\":");
        out.print(t.gps.sats); out.print(",\"lat\":");
        out.print(t.gps.lat, 6); out.print(",\"lon\":");
        out.print(t.gps.lon, 6); out.print(",\"alt\":");
        out.print(t.baro.altitude_m, 2); out.print(",\"v\":");
        out.print(t.power.bus_voltage_v, 2); out.print(",\"i\":");
        out.print(t.power.current_a, 2); out.print(",\"loop\":");
        out.print(t.loop_hz); out.println("}}");
    } else if (strcmp(cmd, "calibrate") == 0) {
        if (sp && strstr(sp + 1, "gyro")) {
            mpu9250_calibrate_gyro();
            out.println("OK giroscopio calibrado");
        } else if (sp && strstr(sp + 1, "mag")) {
            mpu9250_calibrate_mag();
            out.println("OK magnetometro calibrado (girar 2 s)");
        } else {
            out.println("uso: calibrate gyro|mag");
        }
    } else if (strcmp(cmd, "geofence") == 0) {
        geofence_t gf;
        geofence_init(&gf);
        const mission_state_t m = mission_get();
        geofence_state_t gs = geofence_check(&gf, m.home_lat, m.home_lon,
                                             &g_telemetry.gps, &g_telemetry.baro);
        out.print("max_dist="); out.print(gf.max_dist_m, 0);
        out.print(" max_alt="); out.print(gf.max_alt_m, 0);
        out.print(" dist_exceeded="); out.print(gs.dist_exceeded);
        out.print(" alt_exceeded="); out.println(gs.alt_exceeded);
    } else if (strcmp(cmd, "blackbox") == 0) {
        out.print("muestras="); out.println(blackbox_count());
        blackbox_dump(out);
    } else if (strcmp(cmd, "arm") == 0) {
        if (sp && strstr(sp + 1, "confirm")) {
            cmd_arm(out);
        } else {
            out.println("ERROR usa 'arm confirm'");
        }
    } else if (strcmp(cmd, "disarm") == 0) {
        failsafe_disarm();
        esc_disarm();
        out.println("OK desarmado");
    } else if (strcmp(cmd, "oled") == 0) {
        if (sp && strstr(sp + 1, "status")) {
            oled_set_view(OLED_VIEW_STATUS); out.println("OK vista status");
        } else if (sp && strstr(sp + 1, "attitude")) {
            oled_set_view(OLED_VIEW_ATTITUDE); out.println("OK vista attitude");
        } else if (sp && strstr(sp + 1, "baro")) {
            oled_set_view(OLED_VIEW_BARO); out.println("OK vista baro");
        } else if (sp && strstr(sp + 1, "gps")) {
            oled_set_view(OLED_VIEW_GPS); out.println("OK vista gps");
        } else if (sp && strstr(sp + 1, "obstacle")) {
            oled_set_view(OLED_VIEW_OBSTACLE); out.println("OK vista obstacle");
        } else {
            out.println("uso: oled status|attitude|baro|gps|obstacle");
        }
    } else if (strcmp(cmd, "wifi") == 0) {
        if (sp && strstr(sp + 1, "on")) {
            wifi_set_enabled(true);
            out.println("OK wifi on");
        } else if (sp && strstr(sp + 1, "off")) {
            wifi_set_enabled(false);
            out.println("OK wifi off");
        }
    } else if (strcmp(cmd, "reboot") == 0) {
        out.println("reiniciando...");
        delay(50);
        ESP.restart();
    } else {
        out.println("ERROR comando desconocido (help)");
    }
}
