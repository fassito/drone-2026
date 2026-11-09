#pragma once

#include "nav/gps.h"

// ============================================================
// Parser NMEA puro (sin hardware): GPGGA y GPRMC.
// ============================================================

// Procesa una línea NMEA terminada en '\n' (sin el terminador).
// Devuelve true si la línea actualizó el estado (fix o coordenadas).
bool nmea_parse_line(const char *line, gps_state_t *out);
