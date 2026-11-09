#pragma once

// ============================================================
// Filtro de actitud Madgwick AHRS (puro, sin hardware).
// Algoritmo: S. Madgwick, "An efficient orientation filter for
// inertial and inertial/magnetic sensor arrays" (2010).
// ============================================================

typedef struct {
    float q0, q1, q2, q3;   // cuaternión de orientación
    float beta;             // ganancia del paso de gradiente
    float sample_freq;      // Hz
} madgwick_t;

void madgwick_init(madgwick_t *m, float sample_freq, float beta);

// Actualización 6DoF (giroscopio + acelerómetro). g* en rad/s, a* en g.
void madgwick_update_imu(madgwick_t *m, float gx, float gy, float gz,
                         float ax, float ay, float az);

// Actualización 9DoF (añade magnetómetro). m* en unidades arbitrarias
// (se normalizan internamente).
void madgwick_update(madgwick_t *m, float gx, float gy, float gz,
                     float ax, float ay, float az,
                     float mx, float my, float mz);

// Convierte el cuaternión a ángulos de Euler (yaw, pitch, roll) en rad.
void madgwick_get_euler(const madgwick_t *m, float *yaw, float *pitch, float *roll);
