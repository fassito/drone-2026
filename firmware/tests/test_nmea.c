#include "test_util.h"
#include "nav/nmea.h"

void test_nmea(void) {
    test_section("Parser NMEA (componente)");

    // GPGGA con fix.
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        const char *line = "$GPGGA,123519,4025.000,N,00342.000,W,1,08,0.9,650.0,M,48.0,M,,*47";
        CHECK(nmea_parse_line(line, &g) && g.fix, "GPGGA con fix");
        CHECK_NEAR(g.lat, 40.416667f, 0.001f, "latitud N decodificada");
        CHECK_NEAR(g.lon, -3.700000f, 0.001f, "longitud W negativa");
        CHECK(g.sats == 8, "satélites = 8");
        CHECK_NEAR(g.alt, 650.0f, 0.1f, "altitud 650 m");
    }
    // GPGGA sin fix.
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        const char *line = "$GPGGA,123519,4025.000,N,00342.000,W,0,03,2.0,,,,M,,M,,*59";
        nmea_parse_line(line, &g);
        CHECK(!g.fix, "GPGGA sin fix");
    }
    // Hemisferio sur / oeste → negativo.
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        const char *line = "$GPGGA,000000,3335.500,S,07040.000,W,1,05,1.0,500.0,M,,,,*00";
        nmea_parse_line(line, &g);
        CHECK_NEAR(g.lat, -33.591667f, 0.001f, "latitud S negativa");
        CHECK_NEAR(g.lon, -70.666667f, 0.001f, "longitud W negativa");
    }
    // GPRMC activo con velocidad y rumbo.
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        const char *line = "$GPRMC,123519,A,4025.000,N,00342.000,W,10.0,180.0,080126,,,A*00";
        CHECK(nmea_parse_line(line, &g) && g.fix, "GPRMC estado A");
        CHECK_NEAR(g.speed, 10.0f * 0.514444f, 0.01f, "velocidad nudos→m/s");
        CHECK_NEAR(g.heading, 180.0f, 0.1f, "rumbo 180°");
    }
    // GPRMC inválido (V).
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        const char *line = "$GPRMC,123519,V,4025.000,N,00342.000,W,,,,080126,,,N*00";
        nmea_parse_line(line, &g);
        CHECK(!g.fix, "GPRMC estado V sin fix");
    }
    // Línea sin '$' → no procesa.
    {
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        CHECK(!nmea_parse_line("sin sentencia", &g), "línea sin $ rechazada");
    }
}
