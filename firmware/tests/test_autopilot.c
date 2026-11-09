#include "test_util.h"
#include "nav/autopilot.h"

static void fill_gps(gps_state_t *g, int fix, float lat, float lon) {
    memset(g, 0, sizeof(*g));
    g->fix = fix;
    g->lat = lat;
    g->lon = lon;
}

static void fill_baro(baro_state_t *b, int valid, float alt) {
    memset(b, 0, sizeof(*b));
    b->valid = valid;
    b->altitude_m = alt;
}

void test_autopilot(void) {
    test_section("Piloto automático (componente)");

    autopilot_t ap;
    autopilot_init(&ap);

    // Misión activa al norte, yaw orientado al norte → avanza, heading_error ≈ 0.
    {
        gps_state_t g;
        baro_state_t b;
        attitude_t att;
        mission_state_t m;
        obstacle_correction_t obs;

        memset(&att, 0, sizeof(att));
        memset(&m, 0, sizeof(m));
        memset(&obs, 0, sizeof(obs));
        obs.speed_scale = 1.0f;
        fill_gps(&g, 1, 40.0f, -3.7f);
        fill_baro(&b, 1, ap.target_alt_m);
        m.active = true;
        m.target_lat = 40.01f;   // ~1.1 km al norte
        m.target_lon = -3.7f;

        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK_NEAR(he, 0.0f, 0.01f, "orientado al norte → heading_error ≈ 0");
        CHECK(pitch > 0.0f, "avanza con pitch > 0");
        CHECK_NEAR(pitch, ap.max_tilt_rad, 1e-3f, "pitch saturado a max_tilt");
        CHECK_NEAR(thr, ap.alt.hover_throttle, 1e-3f, "throttle = hover con altitud correcta");
    }

    // Parada de emergencia por obstáculo.
    {
        gps_state_t g; baro_state_t b; attitude_t att; mission_state_t m; obstacle_correction_t obs;
        memset(&att, 0, sizeof(att)); memset(&m, 0, sizeof(m)); memset(&obs, 0, sizeof(obs));
        fill_gps(&g, 1, 40.0f, -3.7f); fill_baro(&b, 1, ap.target_alt_m);
        m.active = true; m.target_lat = 40.01f; m.target_lon = -3.7f;
        obs.emergency_stop = true; obs.speed_scale = 0.0f;
        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK_NEAR(pitch, 0.0f, 1e-6f, "emergencia → pitch 0");
        CHECK_NEAR(thr, ap.alt.hover_throttle, 1e-6f, "emergencia → throttle hover");
    }

    // Esquiva lateral.
    {
        gps_state_t g; baro_state_t b; attitude_t att; mission_state_t m; obstacle_correction_t obs;
        memset(&att, 0, sizeof(att)); memset(&m, 0, sizeof(m)); memset(&obs, 0, sizeof(obs));
        fill_gps(&g, 1, 40.0f, -3.7f); fill_baro(&b, 1, ap.target_alt_m);
        m.active = true; m.target_lat = 40.01f; m.target_lon = -3.7f;
        obs.avoid_dir = 1; obs.speed_scale = 0.0f;
        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK_NEAR(roll, ap.dodge_tilt_rad, 1e-6f, "esquiva a la derecha");
        CHECK_NEAR(pitch, 0.0f, 1e-6f, "no avanza mientras esquiva");
    }

    // Sin fix → mantener posición (pitch 0).
    {
        gps_state_t g; baro_state_t b; attitude_t att; mission_state_t m; obstacle_correction_t obs;
        memset(&att, 0, sizeof(att)); memset(&m, 0, sizeof(m)); memset(&obs, 0, sizeof(obs));
        fill_gps(&g, 0, 0, 0); fill_baro(&b, 1, ap.target_alt_m);
        m.active = true; m.target_lat = 40.01f; m.target_lon = -3.7f;
        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK_NEAR(pitch, 0.0f, 1e-6f, "sin fix → mantener posición");
    }

    // Control de altitud: barómetro por debajo del objetivo → más throttle.
    {
        gps_state_t g; baro_state_t b; attitude_t att; mission_state_t m; obstacle_correction_t obs;
        memset(&att, 0, sizeof(att)); memset(&m, 0, sizeof(m)); memset(&obs, 0, sizeof(obs));
        fill_gps(&g, 0, 0, 0); fill_baro(&b, 1, 1.0f);   // 1 m por debajo de 2 m
        float roll, pitch, he, thr;
        autopilot_update(&ap, &g, &b, &att, &m, &obs, 0.001f, &roll, &pitch, &he, &thr);
        CHECK(thr > ap.alt.hover_throttle, "bajo el objetivo → más throttle");
    }
}
