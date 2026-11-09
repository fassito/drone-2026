#include "nav/geofence.h"
#include "nav/nav_math.h"

void geofence_init(geofence_t *gf) {
    gf->max_dist_m = 200.0f;
    gf->max_alt_m = 50.0f;
}

geofence_state_t geofence_check(const geofence_t *gf,
                                float home_lat, float home_lon,
                                const gps_state_t *gps,
                                const baro_state_t *baro) {
    geofence_state_t s;
    s.dist_exceeded = false;
    s.alt_exceeded = false;

    const bool home_valid = !(home_lat == 0.0f && home_lon == 0.0f);
    if (gps->fix && home_valid) {
        const float d = haversine_distance_m(home_lat, home_lon, gps->lat, gps->lon);
        s.dist_exceeded = d > gf->max_dist_m;
    }
    if (baro && baro->valid) {
        s.alt_exceeded = baro->altitude_m > gf->max_alt_m;
    }
    return s;
}
