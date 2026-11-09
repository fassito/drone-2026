#include "safety/failsafe.h"

static failsafe_state_t state;

void failsafe_init(void) {
    state.mode = FS_DISARMED;
    state.rx_link = false;
    state.imu_link = false;
    state.throttle_min = false;
    state.battery_low = false;
    state.overcurrent = false;
}

void failsafe_tick(void) {
    if (state.mode == FS_ARMED) {
        // Cualquier pérdida de enlace o fallo de energía en vuelo => corte.
        if (!state.rx_link || !state.imu_link ||
            state.battery_low || state.overcurrent) {
            state.mode = FS_FAILSAFE;
        }
    }
}

failsafe_state_t failsafe_get_state(void) {
    return state;
}

bool failsafe_arm_request(void) {
    if (state.mode != FS_DISARMED) {
        return false;
    }
    // Condiciones de armado: enlaces OK, throttle mínimo y sin fallo de energía.
    if (state.rx_link && state.imu_link && state.throttle_min &&
        !state.battery_low && !state.overcurrent) {
        state.mode = FS_ARMED;
        return true;
    }
    return false;
}

void failsafe_disarm(void) {
    state.mode = FS_DISARMED;
}

void failsafe_set_rx_link(bool ok) {
    state.rx_link = ok;
}

void failsafe_set_imu_link(bool ok) {
    state.imu_link = ok;
}

void failsafe_set_throttle_min(bool is_min) {
    state.throttle_min = is_min;
}

void failsafe_set_battery_low(bool low) {
    state.battery_low = low;
}

void failsafe_set_overcurrent(bool over) {
    state.overcurrent = over;
}
