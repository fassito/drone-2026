#include "sensors/madgwick.h"

#include <math.h>

static float inv_sqrt(float x) {
    return 1.0f / sqrtf(x);
}

void madgwick_init(madgwick_t *m, float sample_freq, float beta) {
    m->q0 = 1.0f;
    m->q1 = 0.0f;
    m->q2 = 0.0f;
    m->q3 = 0.0f;
    m->sample_freq = sample_freq;
    m->beta = beta;
}

void madgwick_update_imu(madgwick_t *m, float gx, float gy, float gz,
                         float ax, float ay, float az) {
    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot1, q_dot2, q_dot3, q_dot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2;
    float q0q0, q1q1, q2q2, q3q3;

    // Tasa de cambio del cuaternión a partir del giroscopio.
    q_dot1 = 0.5f * (-m->q1 * gx - m->q2 * gy - m->q3 * gz);
    q_dot2 = 0.5f * (m->q0 * gx + m->q2 * gz - m->q3 * gy);
    q_dot3 = 0.5f * (m->q0 * gy - m->q1 * gz + m->q3 * gx);
    q_dot4 = 0.5f * (m->q0 * gz + m->q1 * gy - m->q2 * gx);

    // Paso correctivo solo si el acelerómetro es válido.
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm;
        ay *= recip_norm;
        az *= recip_norm;

        _2q0 = 2.0f * m->q0; _2q1 = 2.0f * m->q1;
        _2q2 = 2.0f * m->q2; _2q3 = 2.0f * m->q3;
        _4q0 = 4.0f * m->q0; _4q1 = 4.0f * m->q1; _4q2 = 4.0f * m->q2;
        _8q1 = 8.0f * m->q1; _8q2 = 8.0f * m->q2;
        q0q0 = m->q0 * m->q0; q1q1 = m->q1 * m->q1;
        q2q2 = m->q2 * m->q2; q3q3 = m->q3 * m->q3;

        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * m->q1 - _2q0 * ay
           - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * m->q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay
           - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * m->q3 - _2q1 * ax + 4.0f * q2q2 * m->q3 - _2q2 * ay;

        const float ss = s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3;
        if (ss > 1e-12f) {
            recip_norm = inv_sqrt(ss);
            s0 *= recip_norm;
            s1 *= recip_norm;
            s2 *= recip_norm;
            s3 *= recip_norm;

            q_dot1 -= m->beta * s0;
            q_dot2 -= m->beta * s1;
            q_dot3 -= m->beta * s2;
            q_dot4 -= m->beta * s3;
        }
    }

    const float dt = 1.0f / m->sample_freq;
    m->q0 += q_dot1 * dt;
    m->q1 += q_dot2 * dt;
    m->q2 += q_dot3 * dt;
    m->q3 += q_dot4 * dt;

    recip_norm = inv_sqrt(m->q0 * m->q0 + m->q1 * m->q1 + m->q2 * m->q2 + m->q3 * m->q3);
    m->q0 *= recip_norm;
    m->q1 *= recip_norm;
    m->q2 *= recip_norm;
    m->q3 *= recip_norm;
}

void madgwick_update(madgwick_t *m, float gx, float gy, float gz,
                     float ax, float ay, float az,
                     float mx, float my, float mz) {
    // Sin magnetómetro válido → usar el filtro 6DoF.
    if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
        madgwick_update_imu(m, gx, gy, gz, ax, ay, az);
        return;
    }

    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot1, q_dot2, q_dot3, q_dot4;
    float hx, hy;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx;
    float _2bx, _2bz, _4bx, _4bz;
    float _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3;
    float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    q_dot1 = 0.5f * (-m->q1 * gx - m->q2 * gy - m->q3 * gz);
    q_dot2 = 0.5f * (m->q0 * gx + m->q2 * gz - m->q3 * gy);
    q_dot3 = 0.5f * (m->q0 * gy - m->q1 * gz + m->q3 * gx);
    q_dot4 = 0.5f * (m->q0 * gz + m->q1 * gy - m->q2 * gx);

    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
        ax *= recip_norm; ay *= recip_norm; az *= recip_norm;

        recip_norm = inv_sqrt(mx * mx + my * my + mz * mz);
        mx *= recip_norm; my *= recip_norm; mz *= recip_norm;

        _2q0mx = 2.0f * m->q0 * mx;
        _2q0my = 2.0f * m->q0 * my;
        _2q0mz = 2.0f * m->q0 * mz;
        _2q1mx = 2.0f * m->q1 * mx;
        _2q0 = 2.0f * m->q0; _2q1 = 2.0f * m->q1;
        _2q2 = 2.0f * m->q2; _2q3 = 2.0f * m->q3;
        _2q0q2 = 2.0f * m->q0 * m->q2;
        _2q2q3 = 2.0f * m->q2 * m->q3;
        q0q0 = m->q0 * m->q0; q0q1 = m->q0 * m->q1;
        q0q2 = m->q0 * m->q2; q0q3 = m->q0 * m->q3;
        q1q1 = m->q1 * m->q1; q1q2 = m->q1 * m->q2;
        q1q3 = m->q1 * m->q3; q2q2 = m->q2 * m->q2;
        q2q3 = m->q2 * m->q3; q3q3 = m->q3 * m->q3;

        hx = mx * q0q0 - _2q0my * m->q3 + _2q0mz * m->q2 + mx * q1q1
           + _2q1 * my * m->q2 + _2q1 * mz * m->q3 - mx * q2q2 - mx * q3q3;
        hy = _2q0mx * m->q3 + my * q0q0 - _2q0mz * m->q1 + _2q1mx * m->q2
           - my * q1q1 + my * q2q2 + _2q2 * mz * m->q3 - my * q3q3;
        _2bx = sqrtf(hx * hx + hy * hy);
        _2bz = -_2q0mx * m->q2 + _2q0my * m->q1 + mz * q0q0 + _2q1mx * m->q3
             - mz * q1q1 + _2q2 * my * m->q3 - mz * q2q2 + mz * q3q3;
        _4bx = 2.0f * _2bx;
        _4bz = 2.0f * _2bz;

        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax)
           + _2q1 * (2.0f * q0q1 + _2q2q3 - ay)
           - _2bz * m->q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx)
           + (-_2bx * m->q3 + _2bz * m->q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my)
           + _2bx * m->q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax)
           + _2q0 * (2.0f * q0q1 + _2q2q3 - ay)
           - 4.0f * m->q1 * (1.0f - 2.0f * q1q1 - 2.0f * q2q2 - az)
           + _2bz * m->q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx)
           + (_2bx * m->q2 + _2bz * m->q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my)
           + (_2bx * m->q3 - _4bz * m->q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax)
           + _2q3 * (2.0f * q0q1 + _2q2q3 - ay)
           - 4.0f * m->q2 * (1.0f - 2.0f * q1q1 - 2.0f * q2q2 - az)
           + (-_4bx * m->q2 - _2bz * m->q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx)
           + (_2bx * m->q1 + _2bz * m->q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my)
           + (_2bx * m->q0 - _4bz * m->q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax)
           + _2q2 * (2.0f * q0q1 + _2q2q3 - ay)
           + (-_4bx * m->q3 + _2bz * m->q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx)
           + (-_2bx * m->q0 + _2bz * m->q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my)
           + _2bx * m->q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);

        const float ss = s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3;
        if (ss > 1e-12f) {
            recip_norm = inv_sqrt(ss);
            s0 *= recip_norm; s1 *= recip_norm;
            s2 *= recip_norm; s3 *= recip_norm;

            q_dot1 -= m->beta * s0;
            q_dot2 -= m->beta * s1;
            q_dot3 -= m->beta * s2;
            q_dot4 -= m->beta * s3;
        }
    }

    const float dt = 1.0f / m->sample_freq;
    m->q0 += q_dot1 * dt;
    m->q1 += q_dot2 * dt;
    m->q2 += q_dot3 * dt;
    m->q3 += q_dot4 * dt;

    recip_norm = inv_sqrt(m->q0 * m->q0 + m->q1 * m->q1 + m->q2 * m->q2 + m->q3 * m->q3);
    m->q0 *= recip_norm;
    m->q1 *= recip_norm;
    m->q2 *= recip_norm;
    m->q3 *= recip_norm;
}

void madgwick_get_euler(const madgwick_t *m, float *yaw, float *pitch, float *roll) {
    const float q0 = m->q0, q1 = m->q1, q2 = m->q2, q3 = m->q3;
    *roll  = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2));
    *pitch = asinf(2.0f * (q0 * q2 - q3 * q1));
    *yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3));
}
