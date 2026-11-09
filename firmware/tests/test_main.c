#include "test_util.h"

// Declaración de las suites por componente e integración.
void test_pid(void);
void test_mixer(void);
void test_nav_math(void);
void test_nmea(void);
void test_ibus(void);
void test_madgwick(void);
void test_mag_calib(void);
void test_obstacle(void);
void test_mission(void);
void test_geofence(void);
void test_failsafe(void);
void test_autotune(void);
void test_autopilot(void);
void test_altitude(void);
void test_integration(void);

int main(void) {
    printf("Batería de tests Drone-2026 (host, sin hardware)\n");

    test_pid();
    test_mixer();
    test_nav_math();
    test_nmea();
    test_ibus();
    test_madgwick();
    test_mag_calib();
    test_obstacle();
    test_mission();
    test_geofence();
    test_failsafe();
    test_autotune();
    test_autopilot();
    test_altitude();
    test_integration();

    printf("\n%s: %d checks, %d fallos\n",
           g_failures == 0 ? "OK" : "ERRORES", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
