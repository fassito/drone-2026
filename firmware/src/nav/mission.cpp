#include "nav/mission.h"
#include "nav/nav_math.h"

#define WAYPOINT_RADIUS_M 2.0f

static mission_state_t mission;

static void set_target(mission_state_t *m, float lat, float lon) {
    m->target_lat = lat;
    m->target_lon = lon;
}

void mission_init(void) {
    mission.active = false;
    mission.target_lat = 0.0f;
    mission.target_lon = 0.0f;
    mission.wp_count = 0;
    mission.wp_index = 0;
    mission.is_rth = false;
    mission.home_set = false;
    mission.home_lat = 0.0f;
    mission.home_lon = 0.0f;
}

void mission_set_target(float lat, float lon) {
    mission.wp_lat[0] = lat;
    mission.wp_lon[0] = lon;
    mission.wp_count = 1;
    mission.wp_index = 0;
    mission.is_rth = false;
    set_target(&mission, lat, lon);
    mission.active = true;
}

void mission_set_waypoints(const float *lats, const float *lons, uint8_t count) {
    if (count > MISSION_MAX_WAYPOINTS) count = MISSION_MAX_WAYPOINTS;
    for (uint8_t i = 0; i < count; i++) {
        mission.wp_lat[i] = lats[i];
        mission.wp_lon[i] = lons[i];
    }
    mission.wp_count = count;
    mission.wp_index = 0;
    mission.is_rth = false;
    if (count > 0) {
        set_target(&mission, lats[0], lons[0]);
        mission.active = true;
    } else {
        mission.active = false;
    }
}

void mission_go_home(void) {
    if (!mission.home_set) return;
    mission.is_rth = true;
    set_target(&mission, mission.home_lat, mission.home_lon);
    mission.active = true;
}

void mission_stop(void) {
    mission.active = false;
    mission.is_rth = false;
}

mission_state_t mission_get(void) {
    return mission;
}

void mission_set_home(float lat, float lon) {
    mission.home_lat = lat;
    mission.home_lon = lon;
    mission.home_set = true;
}

bool mission_nav_info(const mission_state_t *m, const gps_state_t *gps,
                      float *bearing_rad, float *distance_m) {
    if (!m->active || !gps->fix) return false;

    *distance_m = haversine_distance_m(gps->lat, gps->lon, m->target_lat, m->target_lon);
    *bearing_rad = bearing_to_rad(gps->lat, gps->lon, m->target_lat, m->target_lon);

    if (*distance_m <= WAYPOINT_RADIUS_M) {
        return false;   // objetivo alcanzado
    }
    return true;
}

void mission_advance(const gps_state_t *gps) {
    if (!mission.active || !gps->fix) return;

    const float d = haversine_distance_m(gps->lat, gps->lon, mission.target_lat, mission.target_lon);
    if (d > WAYPOINT_RADIUS_M) return;

    if (mission.is_rth) {
        mission.active = false;   // llegado a casa
        mission.is_rth = false;
        return;
    }

    mission.wp_index++;
    if (mission.wp_index < mission.wp_count) {
        set_target(&mission, mission.wp_lat[mission.wp_index],
                   mission.wp_lon[mission.wp_index]);
    } else {
        mission.active = false;   // misión completada
    }
}
