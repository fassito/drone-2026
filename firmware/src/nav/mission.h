#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "nav/gps.h"

// ============================================================
// Misión autónoma: waypoints (múltiples) y return-to-home.
// Responsable: navigation-engineer. Skill: gps-nav.
// ============================================================

#define MISSION_MAX_WAYPOINTS 8

typedef struct {
    bool    active;      // misión en curso
    // Objetivo actual (apunta al waypoint activo o al home en RTH).
    float   target_lat;
    float   target_lon;
    // Waypoints de la misión.
    float   wp_lat[MISSION_MAX_WAYPOINTS];
    float   wp_lon[MISSION_MAX_WAYPOINTS];
    uint8_t wp_count;
    uint8_t wp_index;
    bool    is_rth;      // true = volviendo a casa
    // Home (posición de armado).
    bool    home_set;
    float   home_lat;
    float   home_lon;
} mission_state_t;

void mission_init(void);

// Fija un único destino (equivale a una misión de 1 waypoint).
void mission_set_target(float lat, float lon);

// Fija una lista de waypoints (se recorren en orden).
void mission_set_waypoints(const float *lats, const float *lons, uint8_t count);

// Activa el return-to-home hacia la posición de armado.
void mission_go_home(void);

void mission_stop(void);
mission_state_t mission_get(void);

// Guarda la posición actual como home (primer armado con fix GPS).
void mission_set_home(float lat, float lon);

// Calcula rumbo (rad, [0,2π)) y distancia (m) desde la posición GPS hacia el
// objetivo activo. Devuelve false si no hay misión activa, no hay fix o el
// objetivo ya está alcanzado.
bool mission_nav_info(const mission_state_t *m, const gps_state_t *gps,
                      float *bearing_rad, float *distance_m);

// Avanza al siguiente waypoint si se ha alcanzado el actual.
// Desactiva la misión al completar el último (o al llegar a home en RTH).
void mission_advance(const gps_state_t *gps);
