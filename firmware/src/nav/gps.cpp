#include "nav/gps.h"
#include "nav/nmea.h"
#include "config.h"

#include <Arduino.h>
#include <SoftwareSerial.h>

static SoftwareSerial gps_serial;
static char line[128];
static uint8_t line_len = 0;
static gps_state_t state;
static uint32_t last_update_ms = 0;

void gps_init(void) {
    gps_serial.begin(GPS_BAUD, SWSERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN, false);
    state.fix = false;
    state.sats = 0;
    state.lat = 0.0f;
    state.lon = 0.0f;
    state.alt = 0.0f;
    state.speed = 0.0f;
    state.heading = 0.0f;
}

gps_state_t gps_read(void) {
    while (gps_serial.available() > 0) {
        const char c = (char)gps_serial.read();
        if (c == '\n') {
            line[line_len] = '\0';
            if (line_len > 0) {
                if (nmea_parse_line(line, &state)) {
                    last_update_ms = millis();
                }
            }
            line_len = 0;
        } else if (line_len < sizeof(line) - 1) {
            line[line_len++] = c;
        } else {
            line_len = 0;   // línea demasiado larga, descartar
        }
    }
    return state;
}
