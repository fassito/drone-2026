#include "radio/ibus_parser.h"

bool ibus_parse_frame(const uint8_t *frame, uint32_t len, float channels[IBUS_CHANNELS]) {
    if (len < IBUS_FRAME_LEN) return false;
    if (frame[0] != 0x20 || frame[1] != 0x40) return false;

    // Checksum: 0xFFFF - suma(bytes[2..30]), little-endian en bytes[30..31].
    uint16_t sum = 0;
    for (int i = 2; i < 30; i++) {
        sum += frame[i];
    }
    uint16_t expected = (uint16_t)(0xFFFF - sum);
    uint16_t actual = (uint16_t)(frame[30] | (frame[31] << 8));
    if (actual != expected) return false;

    for (int i = 0; i < IBUS_CHANNELS; i++) {
        uint16_t raw = (uint16_t)(frame[2 + i * 2] | (frame[3 + i * 2] << 8));
        float value = ((float)raw - 1500.0f) / 500.0f;
        if (value > 1.0f) value = 1.0f;
        if (value < -1.0f) value = -1.0f;
        channels[i] = value;
    }
    return true;
}
