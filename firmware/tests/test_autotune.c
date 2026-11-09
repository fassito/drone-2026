#include "test_util.h"
#include "flight/autotune.h"

void test_autotune(void) {
    test_section("Autoajuste PID (componente)");

    // Reglas de Ziegler-Nichols.
    {
        pid_gains_t g;
        autotune_zn_gains(1.0f, 1.0f, &g);
        CHECK_NEAR(g.kp, 0.6f, 1e-4f, "ZN: Kp = 0.6·Ku");
        CHECK_NEAR(g.ki, 0.6f / 0.5f, 1e-4f, "ZN: Ki = Kp/(0.5·Tu)");
        CHECK_NEAR(g.kd, 0.6f * 0.125f, 1e-4f, "ZN: Kd = Kp·(0.125·Tu)");
    }

    // Máquina de estados: converge con una oscilación sintética.
    {
        autotune_t at;
        pid_state_t rate;
        pid_init(&rate, 0.1f, 0.0f, 0.0f, 0.001f);
        pid_set_limits(&rate, -1.0f, 1.0f, 1.0f, 0.01f);

        autotune_init(&at);
        autotune_start(&at, &rate, 0);

        // Oscilación de 2 Hz, actitud nula.
        for (int i = 0; i < 5000; i++) {
            const float t = i * 0.001f;
            const float rate_meas = sinf(2.0f * 3.14159265f * 2.0f * t);
            autotune_step(&at, &rate, rate_meas, 0.0f, 0.001f);
        }
        CHECK(at.done, "autotune termina");
        CHECK(!at.failed, "autotune sin fallo");
        CHECK(at.half_periods >= 2, "detecta varios semiciclos");
        CHECK_NEAR(rate.gains.kp, at.result.kp, 1e-6f, "ganancias aplicadas al lazo");
        CHECK_NEAR(rate.gains.kp, 0.6f * at.result.kp / 0.6f, 1e-6f, "Kp coherente con Ku");
    }

    // Divergencia: actitud fuera de límite → aborta y restaura.
    {
        autotune_t at;
        pid_state_t rate;
        pid_init(&rate, 0.1f, 0.0f, 0.0f, 0.001f);
        pid_set_limits(&rate, -1.0f, 1.0f, 1.0f, 0.01f);

        autotune_init(&at);
        autotune_start(&at, &rate, 0);
        const pid_gains_t backup = at.backup;

        autotune_step(&at, &rate, 0.0f, 1.0f, 0.001f);   // actitud 1 rad > 0.7
        CHECK(at.failed, "divergencia detectada");
        CHECK(!at.active, "autotune aborta");
        CHECK_NEAR(rate.gains.kp, backup.kp, 1e-6f, "ganancias restauradas");
    }
}
