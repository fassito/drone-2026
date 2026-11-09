#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// GPS Neo 6M-0-001 (NMEA por SoftwareSerial a 9600 baud).
// Responsable: navigation-engineer. Skill: gps-nav.
// ============================================================

// STUB — pendiente de implementar.

typedef struct {
    bool fix;        // fix válido (>= 1)
    uint8_t sats;    // satélites usados
    float lat;       // grados decimales
    float lon;       // grados decimales
    float alt;       // m
    float speed;     // m/s
    float heading;   // grados (course over ground)
} gps_state_t;

void gps_init(void);
gps_state_t gps_read(void);
