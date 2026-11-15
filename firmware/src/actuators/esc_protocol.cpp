#include "actuators/esc_protocol.h"

uint16_t esc_normalized_to_us(float value)
{
    if (!(value > 0.0f)) {          // cubre también NaN
        return ESC_MIN_US;
    }
    if (value >= 1.0f) {
        return ESC_MAX_US;
    }
    const float span = (float)(ESC_MAX_US - ESC_MIN_US);
    return (uint16_t)((float)ESC_MIN_US + value * span + 0.5f);
}

uint16_t esc_ramp_us(uint16_t current_us, uint16_t target_us,
                     uint16_t max_step_us)
{
    if (target_us > current_us) {
        const uint16_t delta = (uint16_t)(target_us - current_us);
        return (delta <= max_step_us) ? target_us
                                      : (uint16_t)(current_us + max_step_us);
    }
    if (target_us < current_us) {
        const uint16_t delta = (uint16_t)(current_us - target_us);
        return (delta <= max_step_us) ? target_us
                                      : (uint16_t)(current_us - max_step_us);
    }
    return target_us;
}

bool esc_pulse_in_range(uint16_t pulse_us)
{
    return pulse_us >= ESC_MIN_US && pulse_us <= ESC_MAX_US;
}
