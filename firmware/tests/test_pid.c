#include "test_util.h"
#include "flight/pid.h"

void test_pid(void) {
    test_section("PID (componente)");

    // Respuesta proporcional.
    {
        pid_state_t p;
        pid_init(&p, 1.0f, 0.0f, 0.0f, 0.001f);
        pid_set_limits(&p, -1.0f, 1.0f, 1.0f, 0.01f);
        CHECK_NEAR(pid_step(&p, 0.0f, -0.5f), 0.5f, 0.001f, "proporcional error 0.5");
    }
    // Integral: rampa lineal con error constante.
    {
        pid_state_t p;
        pid_init(&p, 0.0f, 1.0f, 0.0f, 0.001f);
        pid_set_limits(&p, -10.0f, 10.0f, 10.0f, 0.01f);
        float out = 0.0f;
        for (int i = 0; i < 100; i++) out = pid_step(&p, 0.0f, -1.0f);
        CHECK_NEAR(out, 0.1f, 0.001f, "integral acumula 0.1 en 100 ms");
    }
    // Anti-windup: integral limitada con salida saturada.
    {
        pid_state_t p;
        pid_init(&p, 100.0f, 50.0f, 0.0f, 0.001f);
        pid_set_limits(&p, -0.2f, 0.2f, 0.2f, 0.01f);
        float out = 0.0f;
        for (int i = 0; i < 5000; i++) out = pid_step(&p, 0.0f, -1.0f);
        CHECK_NEAR(out, 0.2f, 0.001f, "anti-windup: salida saturada");
    }
    // Derivada: el término D añade salida ante un escalón.
    {
        pid_state_t pd, pp;
        pid_init(&pd, 1.0f, 0.0f, 0.01f, 0.001f);
        pid_init(&pp, 1.0f, 0.0f, 0.0f, 0.001f);
        pid_set_limits(&pd, -10.0f, 10.0f, 10.0f, 0.01f);
        pid_set_limits(&pp, -10.0f, 10.0f, 10.0f, 0.01f);
        float out_d = pid_step(&pd, 1.0f, 0.0f);
        float out_p = pid_step(&pp, 1.0f, 0.0f);
        CHECK(out_d > out_p, "término derivativo añade salida en escalón");
    }
    // Reset limpia integrador y derivada.
    {
        pid_state_t p;
        pid_init(&p, 1.0f, 1.0f, 1.0f, 0.001f);
        for (int i = 0; i < 100; i++) pid_step(&p, 1.0f, 0.0f);
        pid_reset(&p);
        CHECK_NEAR(p.integral, 0.0f, 1e-6f, "reset limpia integral");
        CHECK_NEAR(p.prev_derivative, 0.0f, 1e-6f, "reset limpia derivada");
    }
}
