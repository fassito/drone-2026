#include "test_util.h"
#include "sensors/madgwick.h"

void test_madgwick(void) {
    test_section("Filtro Madgwick (componente)");

    // Reposo: gravedad +z → roll/pitch ≈ 0.
    {
        madgwick_t m;
        float yaw, pitch, roll;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 2000; i++) {
            madgwick_update_imu(&m, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        CHECK_NEAR(roll * 57.2958f, 0.0f, 2.0f, "reposo: roll ≈ 0");
        CHECK_NEAR(pitch * 57.2958f, 0.0f, 2.0f, "reposo: pitch ≈ 0");
    }
    // Inclinación 30° en roll.
    {
        madgwick_t m;
        float yaw, pitch, roll;
        const float th = 30.0f * 0.0174532925f;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 3000; i++) {
            madgwick_update_imu(&m, 0.0f, 0.0f, 0.0f, 0.0f, sinf(th), cosf(th));
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        CHECK_NEAR(roll * 57.2958f, 30.0f, 3.0f, "roll 30° converge");
    }
    // Inclinación 30° en pitch (magnitud).
    {
        madgwick_t m;
        float yaw, pitch, roll;
        const float th = 30.0f * 0.0174532925f;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 3000; i++) {
            madgwick_update_imu(&m, 0.0f, 0.0f, 0.0f, sinf(th), 0.0f, cosf(th));
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        CHECK_NEAR(fabsf(pitch) * 57.2958f, 30.0f, 3.0f, "pitch 30° converge (magnitud)");
    }
    // Integración de giroscopio puro: giro de 90° en yaw.
    {
        madgwick_t m;
        float yaw, pitch, roll;
        const float wz = 1.57079632679f;   // 90°/s
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 1000; i++) {
            // 1 s girando a 90°/s → 90° de yaw.
            madgwick_update_imu(&m, 0.0f, 0.0f, wz, 0.0f, 0.0f, 1.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        CHECK_NEAR(fabsf(yaw) * 57.2958f, 90.0f, 8.0f, "yaw integra 90°");
    }
    // Cuaternión normalizado.
    {
        madgwick_t m;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 100; i++) {
            madgwick_update_imu(&m, 0.1f, -0.2f, 0.3f, 0.1f, -0.2f, 1.0f);
        }
        const float norm = m.q0*m.q0 + m.q1*m.q1 + m.q2*m.q2 + m.q3*m.q3;
        CHECK_NEAR(norm, 1.0f, 1e-3f, "cuaternión normalizado");
    }

    // 9DoF: el magnetómetro fija el yaw sin deriva.
    {
        madgwick_t m;
        float yaw, pitch, roll;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 2000; i++) {
            // mag norte (a lo largo de X), gravedad +z.
            madgwick_update(&m, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 0.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        const float yaw0 = yaw;
        // Sin deriva significativa en 1000 iteraciones más.
        for (int i = 0; i < 1000; i++) {
            madgwick_update(&m, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 0.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        CHECK_NEAR(fabsf(yaw - yaw0) * 57.2958f, 0.0f, 2.0f, "9DoF: yaw sin deriva");
    }
    // 9DoF: girar el magnetómetro 90° gira el yaw 90°.
    {
        madgwick_t m;
        float yaw, pitch, roll;
        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 30000; i++) {
            madgwick_update(&m, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            1.0f, 0.0f, 0.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        const float yaw_north = yaw;

        madgwick_init(&m, 1000.0f, 0.1f);
        for (int i = 0; i < 30000; i++) {
            // mag este (a lo largo de Y).
            madgwick_update(&m, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 1.0f, 0.0f);
        }
        madgwick_get_euler(&m, &yaw, &pitch, &roll);
        const float delta = fabsf(yaw - yaw_north) * 57.2958f;
        // Diferencia cercana a 90° (módulo 180°).
        CHECK(fabsf(delta - 90.0f) < 10.0f || fabsf(delta - 270.0f) < 10.0f,
              "9DoF: rotación de 90° del mag gira el yaw");
    }
}
