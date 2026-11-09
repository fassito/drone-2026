#include "test_util.h"
#include "radio/ibus_parser.h"

static void build_frame(uint8_t *frame, const uint16_t ch[6]) {
    uint16_t sum = 0;
    memset(frame, 0, IBUS_FRAME_LEN);
    frame[0] = 0x20;
    frame[1] = 0x40;
    for (int i = 0; i < IBUS_CHANNELS; i++) {
        frame[2 + i * 2] = ch[i] & 0xFF;
        frame[3 + i * 2] = ch[i] >> 8;
    }
    for (int i = 2; i < 30; i++) sum += frame[i];
    const uint16_t cs = 0xFFFF - sum;
    frame[30] = cs & 0xFF;
    frame[31] = cs >> 8;
}

void test_ibus(void) {
    test_section("Parser iBus (componente)");

    // Trama válida con canales en el rango.
    {
        uint8_t frame[IBUS_FRAME_LEN];
        float out[IBUS_CHANNELS];
        const uint16_t ch[6] = {1500, 1000, 2000, 1250, 1750, 1500};
        build_frame(frame, ch);
        CHECK(ibus_parse_frame(frame, IBUS_FRAME_LEN, out), "trama válida");
        CHECK_NEAR(out[0], 0.0f, 0.001f, "1500 → 0");
        CHECK_NEAR(out[1], -1.0f, 0.001f, "1000 → -1");
        CHECK_NEAR(out[2], 1.0f, 0.001f, "2000 → +1");
        CHECK_NEAR(out[3], -0.5f, 0.001f, "1250 → -0.5");
        CHECK_NEAR(out[4], 0.5f, 0.001f, "1750 → +0.5");
    }
    // Valores fuera de rango se limitan a ±1.
    {
        uint8_t frame[IBUS_FRAME_LEN];
        float out[IBUS_CHANNELS];
        const uint16_t ch[6] = {500, 2500, 1500, 1500, 1500, 1500};
        build_frame(frame, ch);
        ibus_parse_frame(frame, IBUS_FRAME_LEN, out);
        CHECK_NEAR(out[0], -1.0f, 0.001f, "500 → satura a -1");
        CHECK_NEAR(out[1], 1.0f, 0.001f, "2500 → satura a +1");
    }
    // Checksum corrupto → rechazar.
    {
        uint8_t frame[IBUS_FRAME_LEN];
        float out[IBUS_CHANNELS];
        const uint16_t ch[6] = {1500,1500,1500,1500,1500,1500};
        build_frame(frame, ch);
        frame[31] ^= 0xFF;   // corromper checksum
        CHECK(!ibus_parse_frame(frame, IBUS_FRAME_LEN, out), "checksum corrupto rechazado");
    }
    // Cabecera incorrecta → rechazar.
    {
        uint8_t frame[IBUS_FRAME_LEN];
        float out[IBUS_CHANNELS];
        const uint16_t ch[6] = {1500,1500,1500,1500,1500,1500};
        build_frame(frame, ch);
        frame[0] = 0xAA;
        CHECK(!ibus_parse_frame(frame, IBUS_FRAME_LEN, out), "cabecera incorrecta rechazada");
    }
    // Trama corta → rechazar.
    {
        uint8_t frame[IBUS_FRAME_LEN];
        float out[IBUS_CHANNELS];
        const uint16_t ch[6] = {1500,1500,1500,1500,1500,1500};
        build_frame(frame, ch);
        CHECK(!ibus_parse_frame(frame, 31, out), "trama corta rechazada");
    }
}
