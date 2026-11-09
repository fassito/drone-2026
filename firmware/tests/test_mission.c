#include "test_util.h"
#include "nav/mission.h"

void test_mission(void) {
    test_section("Misión / RTH (componente)");

    mission_init();

    // Sin misión activa ni fix → nav_info false.
    {
        mission_state_t m = mission_get();
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        g.fix = true;
        float b, d;
        CHECK(!mission_nav_info(&m, &g, &b, &d), "sin misión → sin navegación");
    }

    // Misión activa + fix → distancia y bearing.
    {
        mission_set_target(40.5f, -3.7f);
        mission_state_t m = mission_get();
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        g.fix = true;
        g.lat = 40.0f;
        g.lon = -3.7f;
        float b, d;
        CHECK(mission_nav_info(&m, &g, &b, &d), "misión activa + fix → navegación");
        CHECK_NEAR(d, 55597.0f, 300.0f, "distancia ≈ 55.6 km (0.5° al norte)");
        CHECK_NEAR(b * 57.295779513f, 0.0f, 1.0f, "bearing al norte ≈ 0°");
    }

    // Destino alcanzado (dentro del radio) → nav_info false.
    {
        mission_set_target(40.00001f, -3.7f);
        mission_state_t m = mission_get();
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        g.fix = true;
        g.lat = 40.0f;
        g.lon = -3.7f;
        float b, d;
        CHECK(!mission_nav_info(&m, &g, &b, &d), "destino alcanzado → sin navegación");
    }

    // Sin fix GPS → nav_info false.
    {
        mission_set_target(40.5f, -3.7f);
        mission_state_t m = mission_get();
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        g.fix = false;
        float b, d;
        CHECK(!mission_nav_info(&m, &g, &b, &d), "sin fix → sin navegación");
    }

    // RTH usa la posición de armado guardada.
    {
        mission_set_home(40.0f, -3.7f);
        mission_set_target(41.0f, -3.7f);
        mission_go_home();
        mission_state_t m = mission_get();
        CHECK(m.active, "RTH activa la misión");
        CHECK_NEAR(m.target_lat, 40.0f, 1e-6f, "RTH apunta al home (lat)");
        CHECK_NEAR(m.target_lon, -3.7f, 1e-6f, "RTH apunta al home (lon)");
    }

    // mission_stop desactiva.
    {
        mission_stop();
        mission_state_t m = mission_get();
        CHECK(!m.active, "mission stop desactiva");
    }

    // Múltiples waypoints: avance secuencial.
    {
        const float lats[3] = {40.1f, 40.2f, 40.3f};
        const float lons[3] = {-3.7f, -3.7f, -3.7f};
        mission_set_waypoints(lats, lons, 3);
        mission_state_t m = mission_get();
        CHECK(m.active && m.wp_count == 3 && m.wp_index == 0, "misión de 3 waypoints");

        // Objetivo actual = waypoint 0.
        gps_state_t g;
        memset(&g, 0, sizeof(g));
        g.fix = true; g.lat = 40.0f; g.lon = -3.7f;
        CHECK_NEAR(m.target_lat, 40.1f, 1e-6f, "objetivo actual = wp0");

        // Alcanzar wp0 → avanza a wp1.
        g.lat = 40.1f; g.lon = -3.7f;
        mission_advance(&g);
        m = mission_get();
        CHECK(m.wp_index == 1, "avanza a wp1");
        CHECK_NEAR(m.target_lat, 40.2f, 1e-6f, "objetivo = wp1");

        // Alcanzar wp1 → wp2; alcanzar wp2 → misión completada.
        g.lat = 40.2f; mission_advance(&g);
        g.lat = 40.3f; mission_advance(&g);
        m = mission_get();
        CHECK(!m.active, "misión completada tras el último waypoint");
    }
}
