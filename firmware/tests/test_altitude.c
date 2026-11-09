#include "test_util.h"
#include "nav/altitude_control.h"

void test_altitude(void) {
    test_section("Control de altitud (componente)");

    // Por debajo del objetivo → más throttle.
    {
        altitude_controller_t ac;
        altitude_control_init(&ac);
        float thr = 0.0f;
        altitude_control_update(&ac, 2.0f, 0.0f, 0.001f, &thr);
        CHECK(thr > ac.hover_throttle, "bajo el objetivo → más throttle");
    }
    // Por encima del objetivo → menos throttle.
    {
        altitude_controller_t ac;
        altitude_control_init(&ac);
        float thr = 0.0f;
        altitude_control_update(&ac, 2.0f, 5.0f, 0.001f, &thr);
        CHECK(thr < ac.hover_throttle, "sobre el objetivo → menos throttle");
    }
    // En el objetivo → ~hover.
    {
        altitude_controller_t ac;
        altitude_control_init(&ac);
        float thr = 0.0f;
        altitude_control_update(&ac, 2.0f, 2.0f, 0.001f, &thr);
        CHECK_NEAR(thr, ac.hover_throttle, 0.02f, "en el objetivo → hover");
    }
    // Salida limitada a [0,1].
    {
        altitude_controller_t ac;
        altitude_control_init(&ac);
        float thr = 0.0f;
        for (int i = 0; i < 100; i++) {
            altitude_control_update(&ac, 100.0f, 0.0f, 0.001f, &thr);
        }
        CHECK(thr >= 0.0f && thr <= 1.0f, "throttle limitado a [0,1]");
    }
    // Ajuste del hover throttle.
    {
        altitude_controller_t ac;
        altitude_control_init(&ac);
        altitude_control_set_hover(&ac, 0.55f);
        CHECK_NEAR(ac.hover_throttle, 0.55f, 1e-6f, "hover throttle ajustable");
    }
}
