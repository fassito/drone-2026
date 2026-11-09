#pragma once

#include "nav/gps.h"
#include "sensors/bmp388.h"

// ============================================================
// Geofence: límites de distancia desde home y de altitud.
// Funciones puras, testeables.
// ============================================================

typedef struct {
    float max_dist_m;
    float max_alt_m;
} geofence_t;

typedef struct {
    bool dist_exceeded;
    bool alt_exceeded;
} geofence_state_t;

void geofence_init(geofence_t *gf);

// Evalúa los límites. Home (0,0) o sin fix GPS ⇒ sin violación de distancia.
geofence_state_t geofence_check(const geofence_t *gf,
                                float home_lat, float home_lon,
                                const gps_state_t *gps,
                                const baro_state_t *baro);
