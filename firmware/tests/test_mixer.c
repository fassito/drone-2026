#include "test_util.h"
#include "flight/mixer.h"

static int all_close(const float *a, const float *b, int n, float tol) {
    for (int i = 0; i < n; i++) {
        if (fabsf(a[i] - b[i]) > tol) return 0;
    }
    return 1;
}

void test_mixer(void) {
    test_section("Mixer quad X (componente)");

    // Sin comandos: todos los motores reciben el throttle.
    {
        float out[4];
        mixer_apply(0.0f, 0.0f, 0.0f, 0.5f, out);
        float expect[4] = {0.5f, 0.5f, 0.5f, 0.5f};
        CHECK(all_close(out, expect, 4, 1e-6f), "sin comandos: todos = throttle");
    }
    // Roll derecha: izquierda sube, derecha baja.
    {
        float out[4];
        mixer_apply(0.3f, 0.0f, 0.0f, 0.5f, out);
        CHECK(out[2] > out[0] && out[1] > out[3], "roll derecha: izquierda > derecha");
    }
    // Pitch adelante: traseros suben, delanteros bajan.
    {
        float out[4];
        mixer_apply(0.0f, 0.3f, 0.0f, 0.5f, out);
        CHECK(out[1] > out[0] && out[3] > out[2], "pitch adelante: traseros > delanteros");
    }
    // Yaw derecha: motores CCW suben, CW bajan.
    {
        float out[4];
        mixer_apply(0.0f, 0.0f, 0.3f, 0.5f, out);
        CHECK(out[2] > out[0] && out[3] > out[1], "yaw derecha: CCW suben, CW bajan");
    }
    // Saturación a [0,1].
    {
        float out[4];
        mixer_apply(1.0f, 1.0f, 1.0f, 1.0f, out);
        int ok = 1;
        for (int i = 0; i < 4; i++) if (out[i] > 1.0f || out[i] < 0.0f) ok = 0;
        CHECK(ok, "salida limitada a [0,1]");
    }
    // Throttle 0 con mandos 0 → motores 0.
    {
        float out[4];
        mixer_apply(0.0f, 0.0f, 0.0f, 0.0f, out);
        CHECK(all_close(out, (float[4]){0,0,0,0}, 4, 1e-6f), "throttle 0 → motores 0");
    }
}
