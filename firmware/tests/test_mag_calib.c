#include "test_util.h"
#include "sensors/mag_calib.h"

void test_mag_calib(void) {
    test_section("Calibración magnetómetro (componente)");

    // Hard iron: offset = (min+max)/2.
    {
        float mn[3] = {-100.0f, 50.0f, 200.0f};
        float mx[3] = {100.0f, 150.0f, 400.0f};
        float off[3];
        mag_hard_iron_offsets(mn, mx, off);
        CHECK_NEAR(off[0], 0.0f, 1e-4f, "offset X centrado");
        CHECK_NEAR(off[1], 100.0f, 1e-4f, "offset Y");
        CHECK_NEAR(off[2], 300.0f, 1e-4f, "offset Z");
    }
    // Aplicación de calibración.
    {
        float raw[3] = {200.0f, 300.0f, 600.0f};
        float off[3] = {100.0f, 100.0f, 300.0f};
        float scl[3] = {1.0f, 2.0f, 0.5f};
        float out[3];
        mag_apply_cal(raw, off, scl, out);
        CHECK_NEAR(out[0], 100.0f, 1e-4f, "cal X = (200-100)*1");
        CHECK_NEAR(out[1], 400.0f, 1e-4f, "cal Y = (300-100)*2");
        CHECK_NEAR(out[2], 150.0f, 1e-4f, "cal Z = (600-300)*0.5");
    }
    // Escala normaliza al mayor radio.
    {
        float mn[3] = {-50.0f, -100.0f, -25.0f};
        float mx[3] = {50.0f, 100.0f, 25.0f};
        float scl[3];
        mag_compute_scale(mn, mx, scl);
        // radio X=50, Y=100 (máx), Z=25 → scl X=2, Y=1, Z=4.
        CHECK_NEAR(scl[0], 2.0f, 1e-4f, "escala X");
        CHECK_NEAR(scl[1], 1.0f, 1e-4f, "escala Y (mayor radio)");
        CHECK_NEAR(scl[2], 4.0f, 1e-4f, "escala Z");
    }
}
