#include "test_util.h"
#include "flight/flight_control.h"
#include "nav/autopilot.h"
#include "nav/obstacle_logic.h"
#include "safety/failsafe.h"

// ------------------------------------------------------------
// Planta simulada de cuadricóptero (para integración).
// rate = respuesta de primer orden al comando de motor;
// attitude = integral del rate.
// ------------------------------------------------------------
typedef struct {
    float rate;      // rad/s
    float attitude;  // rad
} plant_t;

static void plant_step(plant_t *p, float motor_cmd, float dt) {
    // motor_cmd en [-1,1] → rate objetivo ±20 rad/s.
    const float target = motor_cmd * 20.0f;
    p->rate += (target - p->rate) * 10.0f * dt;   // tau = 100 ms
    p->attitude += p->rate * dt;
}

void test_integration(void) {
    test_section("Integración: control en cascada + planta");

    // 1) Control de ángulo en cascada converge a un setpoint de roll.
    {
        flight_control_t fc;
        flight_control_init(&fc);

        plant_t plant;
        memset(&plant, 0, sizeof(plant));

        attitude_t att;
        memset(&att, 0, sizeof(att));
        att.valid = true;

        const float setpoint = 0.10f;   // 5.7°
        float motor[4] = {0, 0, 0, 0};

        for (int i = 0; i < 6000; i++) {
            // Medición de la planta.
            att.roll = plant.attitude;
            att.gyro_x = plant.rate;

            flight_control_update(&fc, FLIGHT_MODE_ANGLE,
                                  setpoint, 0.0f, 0.0f, 0.3f, &att, motor);

            // motor[0] (FR) domina el roll; usar el diferencial de roll.
            const float roll_cmd = 0.5f * (motor[2] + motor[1] - motor[0] - motor[3]);
            plant_step(&plant, roll_cmd, 0.001f);
        }
        CHECK_NEAR(plant.attitude, setpoint, 0.02f, "cascada converge a setpoint de roll");
    }

    // 2) Respuesta a un escalón de rate en modo RATE.
    {
        flight_control_t fc;
        flight_control_init(&fc);

        plant_t plant;
        memset(&plant, 0, sizeof(plant));
        attitude_t att;
        memset(&att, 0, sizeof(att));
        att.valid = true;

        const float rate_sp = 2.0f;   // rad/s
        float motor[4] = {0, 0, 0, 0};

        for (int i = 0; i < 3000; i++) {
            att.gyro_x = plant.rate;
            flight_control_update(&fc, FLIGHT_MODE_RATE,
                                  rate_sp, 0.0f, 0.0f, 0.3f, &att, motor);
            const float roll_cmd = 0.5f * (motor[2] + motor[1] - motor[0] - motor[3]);
            plant_step(&plant, roll_cmd, 0.001f);
        }
        CHECK_NEAR(plant.rate, rate_sp, 0.3f, "lazo de rate sigue setpoint de 2 rad/s");
    }

    test_section("Integración: navegación + obstáculos");

    // 3) La corrección de obstáculo anula el avance de la navegación.
    {
        autopilot_t ap;
        autopilot_init(&ap);

        gps_state_t g; baro_state_t b; attitude_t att; mission_state_t m;
        obstacle_state_t os;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        memset(&att, 0, sizeof(att)); memset(&m, 0, sizeof(m));
        memset(&os, 0, sizeof(os));

        g.fix = true; g.lat = 40.0f; g.lon = -3.7f;
        b.valid = true; b.altitude_m = ap.target_alt_m;
        m.active = true; m.target_lat = 40.01f; m.target_lon = -3.7f;

        // Obstáculo frontal → parada + desvío.
        os.distance[0] = 20.0f; os.distance[1] = 70.0f;
        os.distance[2] = 70.0f; os.distance[3] = 30.0f;
        obstacle_correction_t obs = obstacle_correction(&os);

        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);

        CHECK_NEAR(pitch, 0.0f, 1e-6f, "obstáculo anula avance");
        CHECK(roll != 0.0f, "obstáculo genera desvío");
        CHECK_NEAR(thr, ap.alt.hover_throttle, 1e-3f, "altitud objetivo → hover");

        // Sin obstáculo → reanuda avance.
        os.distance[0] = 80.0f; os.distance[1] = 80.0f;
        os.distance[2] = 80.0f; os.distance[3] = 80.0f;
        obs = obstacle_correction(&os);
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK(pitch > 0.0f, "sin obstáculo → avance");
        CHECK_NEAR(roll, 0.0f, 1e-6f, "sin obstáculo → sin desvío");
    }

    test_section("Integración: failsafe con enlaces");

    // 4) El armado exige radio + IMU + throttle mínimo juntos.
    {
        failsafe_init();
        failsafe_set_rx_link(true);
        failsafe_set_imu_link(true);
        failsafe_set_throttle_min(true);
        CHECK(failsafe_arm_request(), "armado con las 3 condiciones");
        failsafe_set_rx_link(false);
        failsafe_tick();
        CHECK(failsafe_get_state().mode == FS_FAILSAFE, "pérdida de radio en vuelo");
    }
}
