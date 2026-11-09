#include "safety/blackbox.h"
#include "config.h"

static blackbox_sample_t ring[BLACKBOX_MAX_SAMPLES];
static uint16_t head = 0;   // próxima posición a escribir
static uint16_t count = 0;

void blackbox_init(void) {
    head = 0;
    count = 0;
}

void blackbox_record(const blackbox_sample_t *s) {
    ring[head] = *s;
    head = (head + 1) % BLACKBOX_MAX_SAMPLES;
    if (count < BLACKBOX_MAX_SAMPLES) count++;
}

uint16_t blackbox_count(void) {
    return count;
}

void blackbox_dump(Print &out) {
    out.println("t_ms,roll,pitch,yaw,gyro_x,gyro_y,gyro_z,throttle,m0,m1,m2,m3,mode,failsafe");
    const uint16_t start = (count < BLACKBOX_MAX_SAMPLES)
                           ? 0
                           : (head % BLACKBOX_MAX_SAMPLES);
    for (uint16_t i = 0; i < count; i++) {
        const uint16_t idx = (start + i) % BLACKBOX_MAX_SAMPLES;
        const blackbox_sample_t *s = &ring[idx];
        out.print(s->t_ms); out.print(',');
        out.print(s->roll, 4); out.print(',');
        out.print(s->pitch, 4); out.print(',');
        out.print(s->yaw, 4); out.print(',');
        out.print(s->gyro_x, 3); out.print(',');
        out.print(s->gyro_y, 3); out.print(',');
        out.print(s->gyro_z, 3); out.print(',');
        out.print(s->throttle, 3); out.print(',');
        for (int m = 0; m < 4; m++) { out.print(s->motor[m], 3); out.print(','); }
        out.print(s->mode); out.print(',');
        out.println(s->failsafe);
    }
}
