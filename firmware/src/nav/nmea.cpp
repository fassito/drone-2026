#include "nav/nmea.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

// Divide una sentencia NMEA en tokens separados por ','.
// Modifica el buffer (termina cada token con '\0').
// Devuelve el número de tokens encontrados (máx. max_tokens).
static int split_fields(char *line, const char *fields[], int max_fields) {
    int n = 0;
    char *p = line;
    fields[n++] = p;
    while (*p && n < max_fields) {
        if (*p == ',') {
            *p = '\0';
            fields[n++] = p + 1;
        } else if (*p == '*') {
            *p = '\0';
            break;
        }
        p++;
    }
    return n;
}

// Convierte latitud/longitud NMEA (ddmm.mmmm) a grados decimales.
static float nmea_coord_to_deg(const char *value, char hemi) {
    if (!value || !*value) return 0.0f;
    float raw = strtof(value, NULL);
    int deg = (int)(raw / 100.0f);
    float minutes = raw - deg * 100.0f;
    float result = deg + minutes / 60.0f;
    if (hemi == 'S' || hemi == 'W') {
        result = -result;
    }
    return result;
}

// Velocidad en nudos → m/s.
static float knots_to_ms(float kt) { return kt * 0.514444f; }

bool nmea_parse_line(const char *line, gps_state_t *out) {
    if (!line || !out) return false;

    // Copia mutable: split_fields termina los tokens con '\0'.
    char buf[160];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // Buscar el inicio de una sentencia.
    char *dollar = strchr(buf, '$');
    if (!dollar) return false;
    char *talker = dollar + 1;

    const char *fields[20];
    int n = split_fields(talker, fields, 20);
    if (n < 2) return false;

    if (strcmp(fields[0], "GPGGA") == 0 || strcmp(fields[0], "GNGGA") == 0) {
        if (n < 10) return false;
        if (fields[6] && *fields[6] != '0' && *fields[6] != '\0') {
            out->fix = true;
            out->sats = (uint8_t)atoi(fields[7]);
            out->lat = nmea_coord_to_deg(fields[2], fields[3][0]);
            out->lon = nmea_coord_to_deg(fields[4], fields[5][0]);
            out->alt = strtof(fields[9], NULL);
            return true;
        }
        out->fix = false;
        out->sats = (uint8_t)atoi(fields[7]);
        return true;
    }

    if (strcmp(fields[0], "GPRMC") == 0 || strcmp(fields[0], "GNRMC") == 0) {
        if (n < 8) return false;
        if (fields[2] && *fields[2] == 'A') {
            out->lat = nmea_coord_to_deg(fields[3], fields[4][0]);
            out->lon = nmea_coord_to_deg(fields[5], fields[6][0]);
            out->speed = knots_to_ms(strtof(fields[7], NULL));
            if (n > 8 && fields[8] && *fields[8]) {
                out->heading = strtof(fields[8], NULL);
            }
            out->fix = true;
            return true;
        }
        out->fix = false;
        return true;
    }

    return false;
}
