#pragma once

#include <stdbool.h>
#include <stdint.h>

// ============================================================
// IMU MPU-9250 (GY-951) por SPI.
// Entrega actitud fusionada (Madgwick/Mahony en el ESP32) y la
// velocidad angular directa del giroscopio para el lazo de rate.
// Responsable: sensor-engineer. Skill: mpu9250-imu.
// ============================================================

// STUB — pendiente de implementar.

typedef struct {
    float yaw;    // rad
    float pitch;  // rad
    float roll;   // rad
    // Velocidad angular directa del giroscopio (sin derivar):
    float gyro_x; // rad/s (roll)
    float gyro_y; // rad/s (pitch)
    float gyro_z; // rad/s (yaw)
    bool  valid;  // false si hay fallo de lectura (failsafe de sensores)
} attitude_t;

void mpu9250_init(void);

// Devuelve el valor del registro WHO_AM_I (0x71 si está bien) o 0 si no
// inicializado. Útil para el autotest.
uint8_t mpu9250_who_am_i(void);

// Calibra los offsets del giroscopio en reposo.
void mpu9250_calibrate_gyro(void);

// Calibra el magnetómetro (hard iron + escala). Bloquea ~2 s mientras se
// gira la placa en los tres ejes. Guarda el resultado en NVS.
void mpu9250_calibrate_mag(void);

// Magnetómetro calibrado en µT (o false si no hay calibración/datos).
bool mpu9250_mag_valid(void);
void mpu9250_get_mag(float mag[3]);

attitude_t mpu9250_read(void);
