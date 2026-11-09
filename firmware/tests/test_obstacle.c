#include "test_util.h"
#include "nav/obstacle_logic.h"

void test_obstacle(void) {
    test_section("Evitación de obstáculos (componente)");

    // Conversión ADC → cm.
    CHECK_NEAR(sharp_adc_to_cm(10), 80.0f, 0.01f, "ADC bajo → 80 cm (sin obstáculo)");
    CHECK(sharp_adc_to_cm(400) < sharp_adc_to_cm(200), "más ADC → menos distancia");
    CHECK(sharp_adc_to_cm(600) >= 10.0f && sharp_adc_to_cm(600) <= 80.0f, "distancia limitada a [10,80]");

    // Sin obstáculos → velocidad completa.
    {
        obstacle_state_t s = { {80.0f, 80.0f, 80.0f, 80.0f}, false };
        obstacle_correction_t c = obstacle_correction(&s);
        CHECK_NEAR(c.speed_scale, 1.0f, 1e-6f, "sin obstáculo → velocidad 1");
        CHECK(c.avoid_dir == 0, "sin obstáculo → sin desvío");
        CHECK(!c.emergency_stop, "sin obstáculo → sin parada");
    }
    // Obstáculo frontal en rango de parada → frenar y desviar al lado libre.
    {
        obstacle_state_t s = { {20.0f, 70.0f, 70.0f, 30.0f}, true };
        obstacle_correction_t c = obstacle_correction(&s);
        CHECK_NEAR(c.speed_scale, 0.0f, 1e-6f, "frontal cerca → velocidad 0");
        CHECK(c.avoid_dir == 1, "desvía a la derecha (más libre)");
    }
    // Desvío a la izquierda cuando la izquierda está más libre.
    {
        obstacle_state_t s = { {20.0f, 30.0f, 70.0f, 70.0f}, true };
        obstacle_correction_t c = obstacle_correction(&s);
        CHECK(c.avoid_dir == -1, "desvía a la izquierda (más libre)");
    }
    // Aviso (no parada) en rango de precaución.
    {
        obstacle_state_t s = { {50.0f, 80.0f, 80.0f, 80.0f}, false };
        obstacle_correction_t c = obstacle_correction(&s);
        CHECK_NEAR(c.speed_scale, 0.4f, 1e-6f, "frontal a 50 cm → velocidad 0.4");
        CHECK(!c.emergency_stop, "50 cm → sin parada de emergencia");
    }
    // Parada de emergencia si cualquier dirección está en rango crítico.
    {
        obstacle_state_t s = { {80.0f, 10.0f, 80.0f, 80.0f}, true };
        obstacle_correction_t c = obstacle_correction(&s);
        CHECK(c.emergency_stop, "obstáculo crítico lateral → emergencia");
        CHECK_NEAR(c.speed_scale, 0.0f, 1e-6f, "emergencia → velocidad 0");
    }
}
