#include "nav/nav_math.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static const float EARTH_RADIUS_M = 6371000.0f;

static float deg2rad(float d) { return d * (M_PI / 180.0f); }

float haversine_distance_m(float lat1, float lon1, float lat2, float lon2) {
    const float dlat = deg2rad(lat2 - lat1);
    const float dlon = deg2rad(lon2 - lon1);
    const float a = sinf(dlat / 2.0f) * sinf(dlat / 2.0f)
                  + cosf(deg2rad(lat1)) * cosf(deg2rad(lat2))
                  * sinf(dlon / 2.0f) * sinf(dlon / 2.0f);
    return 2.0f * EARTH_RADIUS_M * atan2f(sqrtf(a), sqrtf(1.0f - a));
}

float bearing_to_rad(float lat1, float lon1, float lat2, float lon2) {
    const float rlat1 = deg2rad(lat1);
    const float rlat2 = deg2rad(lat2);
    const float dlon = deg2rad(lon2 - lon1);

    const float y = sinf(dlon) * cosf(rlat2);
    const float x = cosf(rlat1) * sinf(rlat2)
                  - sinf(rlat1) * cosf(rlat2) * cosf(dlon);
    return atan2f(y, x);
}

float wrap_pi(float rad) {
    while (rad > M_PI)  rad -= 2.0f * M_PI;
    while (rad < -M_PI) rad += 2.0f * M_PI;
    return rad;
}

float wrap_2pi(float rad) {
    while (rad >= 2.0f * M_PI) rad -= 2.0f * M_PI;
    while (rad < 0.0f)         rad += 2.0f * M_PI;
    return rad;
}
