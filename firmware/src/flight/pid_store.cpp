#include "flight/pid_store.h"

#include <Preferences.h>

#define PREF_NAMESPACE "drone"

void pid_store_save(const flight_control_t *fc) {
    Preferences prefs;
    prefs.begin(PREF_NAMESPACE, false);

    prefs.putFloat("rr_kp", fc->rate_roll.gains.kp);
    prefs.putFloat("rr_ki", fc->rate_roll.gains.ki);
    prefs.putFloat("rr_kd", fc->rate_roll.gains.kd);
    prefs.putFloat("rp_kp", fc->rate_pitch.gains.kp);
    prefs.putFloat("rp_ki", fc->rate_pitch.gains.ki);
    prefs.putFloat("rp_kd", fc->rate_pitch.gains.kd);
    prefs.putFloat("ry_kp", fc->rate_yaw.gains.kp);
    prefs.putFloat("ry_ki", fc->rate_yaw.gains.ki);
    prefs.putFloat("ry_kd", fc->rate_yaw.gains.kd);

    prefs.end();
}

void pid_store_load(flight_control_t *fc) {
    Preferences prefs;
    if (!prefs.begin(PREF_NAMESPACE, true)) return;

    if (prefs.isKey("rr_kp")) {
        fc->rate_roll.gains.kp = prefs.getFloat("rr_kp");
        fc->rate_roll.gains.ki = prefs.getFloat("rr_ki");
        fc->rate_roll.gains.kd = prefs.getFloat("rr_kd");
        fc->rate_pitch.gains.kp = prefs.getFloat("rp_kp");
        fc->rate_pitch.gains.ki = prefs.getFloat("rp_ki");
        fc->rate_pitch.gains.kd = prefs.getFloat("rp_kd");
        fc->rate_yaw.gains.kp = prefs.getFloat("ry_kp");
        fc->rate_yaw.gains.ki = prefs.getFloat("ry_ki");
        fc->rate_yaw.gains.kd = prefs.getFloat("ry_kd");
    }
    prefs.end();
}
