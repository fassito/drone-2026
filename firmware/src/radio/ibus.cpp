#include "radio/ibus.h"
#include "radio/ibus_parser.h"
#include "config.h"

#include <Arduino.h>
#include <HardwareSerial.h>

// ============================================================
// Receptor FS-IA6B por UART1 (iBus 115200). GPIO34 es solo RX.
// ============================================================

static HardwareSerial *ibus_uart;
static uint8_t frame[IBUS_FRAME_LEN];
static uint8_t frame_idx = 0;
static ibus_state_t state;
static uint32_t last_frame_ms = 0;

#define IBUS_TIMEOUT_MS 250

void ibus_init(void) {
    ibus_uart = &Serial1;
    ibus_uart->begin(IBUS_BAUD, SERIAL_8N1, IBUS_RX_PIN, IBUS_TX_PIN);
    frame_idx = 0;
    state.link_ok = false;
    for (int i = 0; i < IBUS_CHANNELS; i++) {
        state.channel[i] = 0.0f;
    }
}

ibus_state_t ibus_read(void) {
    while (ibus_uart->available() > 0) {
        const uint8_t b = (uint8_t)ibus_uart->read();

        if (frame_idx == 0 && b != 0x20) continue;
        if (frame_idx == 1 && b != 0x40) {
            frame_idx = (b == 0x20) ? 1 : 0;
            continue;
        }

        frame[frame_idx++] = b;
        if (frame_idx >= IBUS_FRAME_LEN) {
            frame_idx = 0;
            float channels[IBUS_CHANNELS];
            if (ibus_parse_frame(frame, IBUS_FRAME_LEN, channels)) {
                for (int i = 0; i < IBUS_CHANNELS; i++) {
                    state.channel[i] = channels[i];
                }
                state.link_ok = true;
                last_frame_ms = millis();
            }
        }
    }

    // Failsafe de receptor: sin trama válida reciente.
    if ((millis() - last_frame_ms) > IBUS_TIMEOUT_MS) {
        state.link_ok = false;
        // No mantener el throttle: forzar valor seguro.
        state.channel[2] = -1.0f;
    }

    return state;
}
