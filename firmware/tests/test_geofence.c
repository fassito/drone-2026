#include "test_util.h"
#include "nav/geofence.h"

void test_geofence(void) {
    test_section("Geofence (componente)");

    geofence_t gf;
    geofence_init(&gf);

    // Sin fix GPS → sin violación de distancia.
    {
        gps_state_t g; baro_state_t b;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        g.fix = false;
        geofence_state_t s = geofence_check(&gf, 40.0f, -3.7f, &g, &b);
        CHECK(!s.dist_exceeded, "sin fix → sin violación de distancia");
    }
    // Dentro del límite.
    {
        gps_state_t g; baro_state_t b;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        g.fix = true; g.lat = 40.001f; g.lon = -3.7f;   // ~111 m del home
        geofence_state_t s = geofence_check(&gf, 40.0f, -3.7f, &g, &b);
        CHECK(!s.dist_exceeded, "dentro del límite → sin violación");
    }
    // Fuera del límite de distancia.
    {
        gps_state_t g; baro_state_t b;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        g.fix = true; g.lat = 40.01f; g.lon = -3.7f;   // ~1.1 km
        geofence_state_t s = geofence_check(&gf, 40.0f, -3.7f, &g, &b);
        CHECK(s.dist_exceeded, "fuera del límite → violación de distancia");
    }
    // Altitud excedida.
    {
        gps_state_t g; baro_state_t b;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        g.fix = true;
        b.valid = true; b.altitude_m = 60.0f;
        geofence_state_t s = geofence_check(&gf, 40.0f, -3.7f, &g, &b);
        CHECK(s.alt_exceeded, "altitud 60 m → violación (máx 50)");
    }
    // Altitud dentro del límite.
    {
        gps_state_t g; baro_state_t b;
        memset(&g, 0, sizeof(g)); memset(&b, 0, sizeof(b));
        g.fix = true;
        b.valid = true; b.altitude_m = 10.0f;
        geofence_state_t s = geofence_check(&gf, 40.0f, -3.7f, &g, &b);
        CHECK(!s.alt_exceeded, "altitud 10 m → sin violación");
    }
}
