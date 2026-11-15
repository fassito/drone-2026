#include "test_util.h"
#include "actuators/esc_protocol.h"

static void test_normalized_to_us(void)
{
    test_section("ESC OneShot125: mando → pulso");
    CHECK(esc_normalized_to_us(0.0f) == 125, "0.0 → 125 µs (mínimo)");
    CHECK(esc_normalized_to_us(1.0f) == 250, "1.0 → 250 µs (máximo)");
    CHECK(esc_normalized_to_us(0.5f) == 188, "0.5 → 188 µs (punto medio)");
    CHECK(esc_normalized_to_us(-0.5f) == 125, "negativo → recortado a 125");
    CHECK(esc_normalized_to_us(1.5f) == 250, ">1 → recortado a 250");
    CHECK(esc_normalized_to_us(0.0f / 0.0f) == 125, "NaN → mínimo (no manda motor)");

    // monotonía
    uint16_t prev = 0;
    bool mono = true;
    for (int i = 0; i <= 100; i++) {
        const uint16_t us = esc_normalized_to_us((float)i / 100.0f);
        if (us < prev) mono = false;
        if (us < 125 || us > 250) mono = false;
        prev = us;
    }
    CHECK(mono, "monótona y siempre dentro de [125,250]");
}

static void test_ramp(void)
{
    test_section("ESC OneShot125: rampa de aceleración");
    CHECK(esc_ramp_us(125, 125, 1) == 125, "sin cambio → igual");
    CHECK(esc_ramp_us(125, 250, 1) == 126, "sube como mucho 1 µs por ciclo");
    CHECK(esc_ramp_us(125, 250, 5) == 130, "respeta el paso máximo de 5 µs");
    CHECK(esc_ramp_us(250, 125, 1) == 249, "baja como mucho 1 µs por ciclo");
    CHECK(esc_ramp_us(124, 125, 1) == 125, "salto de 1 → llega exacto");
    CHECK(esc_ramp_us(200, 190, 50) == 190, "paso grande → alcanza el objetivo");

    // desde mínimo a máximo en 125 ciclos de 1 µs
    uint16_t us = 125;
    int cycles = 0;
    while (us < 250 && cycles < 1000) {
        us = esc_ramp_us(us, 250, 1);
        cycles++;
    }
    CHECK(us == 250 && cycles == 125, "125→250 en 125 ciclos (125 ms a 1 kHz)");
}

static void test_range(void)
{
    test_section("ESC OneShot125: rango válido");
    CHECK(!esc_pulse_in_range(0), "0 µs no es válido (sin señal)");
    CHECK(!esc_pulse_in_range(124), "124 µs fuera de rango");
    CHECK(esc_pulse_in_range(125), "125 µs válido (límite)");
    CHECK(esc_pulse_in_range(188), "188 µs válido");
    CHECK(esc_pulse_in_range(250), "250 µs válido (límite)");
    CHECK(!esc_pulse_in_range(251), "251 µs fuera de rango");
}

void test_esc(void)
{
    test_normalized_to_us();
    test_ramp();
    test_range();
}
