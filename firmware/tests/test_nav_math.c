#include "test_util.h"
#include "nav/nav_math.h"

void test_nav_math(void) {
    test_section("Navegación: haversine y bearing (componente)");

    // 1 grado de latitud ≈ 111.19 km.
    CHECK_NEAR(haversine_distance_m(40.0f, -3.7f, 41.0f, -3.7f), 111195.0f, 500.0f,
               "1 grado de latitud ≈ 111 km");

    // 1 grado de longitud en el ecuador ≈ 111.32 km.
    CHECK_NEAR(haversine_distance_m(0.0f, 0.0f, 0.0f, 1.0f), 111319.0f, 500.0f,
               "1 grado de longitud en ecuador ≈ 111 km");

    // Mismo punto → distancia 0.
    CHECK_NEAR(haversine_distance_m(40.4f, -3.7f, 40.4f, -3.7f), 0.0f, 1.0f,
               "mismo punto → 0 m");

    // Madrid → Barcelona: rumbo noreste (entre 30° y 90°).
    {
        float b = bearing_to_rad(40.4168f, -3.7038f, 41.3874f, 2.1686f);
        float deg = b * 57.295779513f;
        if (deg < 0) deg += 360.0f;
        CHECK(deg > 30.0f && deg < 90.0f, "bearing Madrid→Barcelona al NE");
    }

    // Rumbo hacia el norte: ~0°.
    {
        float b = bearing_to_rad(40.0f, -3.7f, 41.0f, -3.7f);
        float deg = b * 57.295779513f;
        if (deg < 0) deg += 360.0f;
        CHECK_NEAR(deg, 0.0f, 1.0f, "bearing hacia el norte ≈ 0°");
    }

    // wrap_pi / wrap_2pi.
    CHECK_NEAR(wrap_pi(3.5f), 3.5f - 2.0f * 3.14159265f, 0.001f, "wrap_pi(3.5)");
    CHECK_NEAR(wrap_2pi(-1.0f), 2.0f * 3.14159265f - 1.0f, 0.001f, "wrap_2pi(-1)");
}
