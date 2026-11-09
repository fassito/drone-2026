#include "telemetry/hc12_link.h"
#include "config.h"

#include <Arduino.h>
#include <HardwareSerial.h>

// ============================================================
// Radio HC-12 (433/868/915 MHz) en UART2 de hardware.
// Stream transparente para el parser de la CLI.
// ============================================================

static HardwareSerial *hc12;

void hc12_init(void) {
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, LOW);   // modo transparente

    hc12 = &Serial2;
    hc12->begin(HC12_BAUD, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
}

bool hc12_available(void) {
    return hc12->available() > 0;
}

int hc12_read_byte(void) {
    return hc12->read();
}

void hc12_write(const char *data, size_t len) {
    hc12->write((const uint8_t *)data, len);
}

void hc12_set_at_mode(bool enable) {
    digitalWrite(HC12_SET_PIN, enable ? HIGH : LOW);
    if (enable) delay(50);
}
