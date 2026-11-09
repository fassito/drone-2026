#include "sensors/mpu9250.h"
#include "sensors/madgwick.h"
#include "sensors/mag_calib.h"
#include "config.h"

#include <Arduino.h>
#include <SPI.h>
#include <Preferences.h>

#define MPU9250_PWR_MGMT_1   0x6B
#define MPU9250_GYRO_CONFIG  0x1B
#define MPU9250_ACCEL_CONFIG 0x1C
#define MPU9250_CONFIG       0x1A
#define MPU9250_ACCEL_CONFIG2 0x1D
#define MPU9250_ACCEL_XOUT_H 0x3B
#define MPU9250_WHO_AM_I     0x75

// I2C master del MPU-9250 (para leer el AK8963 en modo SPI).
#define MPU9250_USER_CTRL      0x6A
#define MPU9250_I2C_MST_CTRL   0x24
#define MPU9250_I2C_SLV0_ADDR  0x25
#define MPU9250_I2C_SLV0_REG   0x26
#define MPU9250_I2C_SLV0_CTRL  0x27
#define MPU9250_I2C_SLV4_ADDR  0x31
#define MPU9250_I2C_SLV4_REG   0x32
#define MPU9250_I2C_SLV4_DO    0x33
#define MPU9250_I2C_SLV4_CTRL  0x34
#define MPU9250_I2C_MST_STATUS 0x36
#define MPU9250_EXT_SENS_DATA  0x49

#define AK8963_ADDR 0x0C
#define AK8963_ST1   0x02
#define AK8963_XOUT  0x03
#define AK8963_CNTL1 0x0A

// Escalas: ±2000 dps → 16.4 LSB/(deg/s); ±16 g → 2048 LSB/g.
#define GYRO_SCALE  16.4f
#define ACCEL_SCALE 2048.0f
#define MAG_SCALE_UT 0.15f   // µT/LSB (AK8963 16 bits)

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453292519943295f
#endif

static SPISettings spi_settings(MPU9250_SPI_HZ, MSBFIRST, SPI_MODE0);

static madgwick_t filter;
static float gyro_offset[3] = {0.0f, 0.0f, 0.0f};
static float mag_offsets[3] = {0.0f, 0.0f, 0.0f};
static float mag_scale[3] = {1.0f, 1.0f, 1.0f};
static bool mag_calibrated = false;
static bool mag_present = false;
static uint32_t last_us = 0;
static bool initialized = false;
static attitude_t last_att;
static uint8_t who_am_i = 0;

static void cs_low(void)  { digitalWrite(MPU9250_SPI_CS, LOW); }
static void cs_high(void) { digitalWrite(MPU9250_SPI_CS, HIGH); }

static void write_reg(uint8_t reg, uint8_t val) {
    cs_low();
    SPI.transfer(reg & 0x7F);
    SPI.transfer(val);
    cs_high();
}

static uint8_t read_reg(uint8_t reg) {
    uint8_t val;
    cs_low();
    SPI.transfer(reg | 0x80);
    val = SPI.transfer(0x00);
    cs_high();
    return val;
}

static void read_regs(uint8_t reg, uint8_t *buf, uint8_t n) {
    cs_low();
    SPI.transfer(reg | 0x80);
    for (uint8_t i = 0; i < n; i++) {
        buf[i] = SPI.transfer(0x00);
    }
    cs_high();
}

// ---------- AK8963 vía I2C master del MPU-9250 ----------
static void ak8963_write(uint8_t reg, uint8_t val) {
    write_reg(MPU9250_I2C_SLV4_ADDR, AK8963_ADDR);   // escritura (sin flag read)
    write_reg(MPU9250_I2C_SLV4_REG, reg);
    write_reg(MPU9250_I2C_SLV4_DO, val);
    write_reg(MPU9250_I2C_SLV4_CTRL, 0x80);          // enable
    delay(2);
}

static void ak8963_init(void) {
    // Habilitar el I2C master del MPU y 400 kHz.
    write_reg(MPU9250_USER_CTRL, read_reg(MPU9250_USER_CTRL) | 0x20);
    write_reg(MPU9250_I2C_MST_CTRL, 0x0D);

    // AK8963: modo continuo 2 (100 Hz), salida 16 bits.
    ak8963_write(AK8963_CNTL1, 0x16);

    // SLV0: leer 7 bytes (ST1 + X/Y/Z) desde 0x02.
    write_reg(MPU9250_I2C_SLV0_ADDR, AK8963_ADDR | 0x80);
    write_reg(MPU9250_I2C_SLV0_REG, AK8963_ST1);
    write_reg(MPU9250_I2C_SLV0_CTRL, 0x86);   // enable + 6 bytes extra
}

static bool read_mag_raw(float mag[3]) {
    uint8_t buf[7];
    read_regs(MPU9250_EXT_SENS_DATA, buf, 7);

    if (!(buf[0] & 0x01)) return false;   // DRDY no listo
    if (buf[0] & 0x08) return false;      // desbordamiento

    mag[0] = (float)(int16_t)((buf[1] | (buf[2] << 8)));
    mag[1] = (float)(int16_t)((buf[3] | (buf[4] << 8)));
    mag[2] = (float)(int16_t)((buf[5] | (buf[6] << 8)));
    return true;
}

// ---------- Persistencia de calibración en NVS ----------
static void mag_cal_save(void) {
    Preferences p;
    p.begin("drone", false);
    p.putFloat("mg_ox", mag_offsets[0]);
    p.putFloat("mg_oy", mag_offsets[1]);
    p.putFloat("mg_oz", mag_offsets[2]);
    p.putFloat("mg_sx", mag_scale[0]);
    p.putFloat("mg_sy", mag_scale[1]);
    p.putFloat("mg_sz", mag_scale[2]);
    p.putBool("mg_ok", true);
    p.end();
}

static void mag_cal_load(void) {
    Preferences p;
    if (!p.begin("drone", true)) return;
    if (p.isKey("mg_ok")) {
        mag_offsets[0] = p.getFloat("mg_ox");
        mag_offsets[1] = p.getFloat("mg_oy");
        mag_offsets[2] = p.getFloat("mg_oz");
        mag_scale[0] = p.getFloat("mg_sx");
        mag_scale[1] = p.getFloat("mg_sy");
        mag_scale[2] = p.getFloat("mg_sz");
        mag_calibrated = true;
    }
    p.end();
}

void mpu9250_init(void) {
    pinMode(MPU9250_SPI_CS, OUTPUT);
    cs_high();

    SPI.begin(MPU9250_SPI_SCK, MPU9250_SPI_MISO, MPU9250_SPI_MOSI, MPU9250_SPI_CS);
    SPI.beginTransaction(spi_settings);

    write_reg(MPU9250_PWR_MGMT_1, 0x80);   // reset
    delay(100);
    write_reg(MPU9250_PWR_MGMT_1, 0x01);   // reloj PLL giroscopio X
    delay(10);

    write_reg(MPU9250_CONFIG, 0x02);        // DLPF giroscopio 92 Hz
    write_reg(MPU9250_GYRO_CONFIG, 0x18);   // ±2000 dps
    write_reg(MPU9250_ACCEL_CONFIG, 0x18);  // ±16 g
    write_reg(MPU9250_ACCEL_CONFIG2, 0x02); // DLPF acelerómetro 41 Hz

    who_am_i = read_reg(MPU9250_WHO_AM_I);

#if MPU9250_USE_MAG
    ak8963_init();
    // Comprobar presencia con una lectura de prueba.
    float probe[3];
    for (int i = 0; i < 5 && !mag_present; i++) {
        mag_present = read_mag_raw(probe);
        delay(5);
    }
    mag_cal_load();
#endif

    madgwick_init(&filter, (float)LOOP_FREQ_HZ, 0.1f);
    mpu9250_calibrate_gyro();
    initialized = true;
}

uint8_t mpu9250_who_am_i(void) {
    return initialized ? who_am_i : 0;
}

void mpu9250_calibrate_gyro(void) {
    const int samples = 200;
    float sum[3] = {0.0f, 0.0f, 0.0f};
    uint8_t buf[14];

    for (int i = 0; i < samples; i++) {
        read_regs(MPU9250_ACCEL_XOUT_H, buf, 14);
        for (int a = 0; a < 3; a++) {
            int16_t raw = (int16_t)((buf[8 + a * 2] << 8) | buf[9 + a * 2]);
            sum[a] += raw;
        }
        delay(2);
    }
    for (int a = 0; a < 3; a++) {
        gyro_offset[a] = (sum[a] / samples) / GYRO_SCALE * DEG_TO_RAD;
    }
    last_us = micros();
}

void mpu9250_calibrate_mag(void) {
    if (!mag_present) return;

    float mn[3] = { 1e9f,  1e9f,  1e9f};
    float mx[3] = {-1e9f, -1e9f, -1e9f};

    const uint32_t start = millis();
    while (millis() - start < 2000) {
        float m[3];
        if (read_mag_raw(m)) {
            for (int i = 0; i < 3; i++) {
                if (m[i] < mn[i]) mn[i] = m[i];
                if (m[i] > mx[i]) mx[i] = m[i];
            }
        }
        delay(5);
    }

    mag_hard_iron_offsets(mn, mx, mag_offsets);
    mag_compute_scale(mn, mx, mag_scale);
    mag_calibrated = true;
    mag_cal_save();
}

bool mpu9250_mag_valid(void) {
    return mag_present && mag_calibrated;
}

void mpu9250_get_mag(float mag[3]) {
    float raw[3];
    if (!read_mag_raw(raw)) {
        mag[0] = mag[1] = mag[2] = 0.0f;
        return;
    }
    mag_apply_cal(raw, mag_offsets, mag_scale, mag);
    for (int i = 0; i < 3; i++) mag[i] *= MAG_SCALE_UT;
}

attitude_t mpu9250_read(void) {
    if (!initialized) {
        last_att.valid = false;
        return last_att;
    }

    uint8_t buf[14];
    read_regs(MPU9250_ACCEL_XOUT_H, buf, 14);

    int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);
    int16_t gx_raw = (int16_t)((buf[8] << 8) | buf[9]);
    int16_t gy_raw = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t gz_raw = (int16_t)((buf[12] << 8) | buf[13]);

    const float gx = gx_raw / GYRO_SCALE * DEG_TO_RAD - gyro_offset[0];
    const float gy = gy_raw / GYRO_SCALE * DEG_TO_RAD - gyro_offset[1];
    const float gz = gz_raw / GYRO_SCALE * DEG_TO_RAD - gyro_offset[2];
    const float ax = ax_raw / ACCEL_SCALE;
    const float ay = ay_raw / ACCEL_SCALE;
    const float az = az_raw / ACCEL_SCALE;

    const uint32_t now = micros();
    const float dt = (now - last_us) * 1e-6f;
    last_us = now;
    if (dt > 0.0f && dt < 0.01f) {
        filter.sample_freq = 1.0f / dt;
    }

    if (mpu9250_mag_valid()) {
        float mag[3];
        mpu9250_get_mag(mag);
        madgwick_update(&filter, gx, gy, gz, ax, ay, az, mag[0], mag[1], mag[2]);
    } else {
        madgwick_update_imu(&filter, gx, gy, gz, ax, ay, az);
    }

    float yaw, pitch, roll;
    madgwick_get_euler(&filter, &yaw, &pitch, &roll);

    last_att.yaw = yaw;
    last_att.pitch = pitch;
    last_att.roll = roll;
    last_att.gyro_x = gx;
    last_att.gyro_y = gy;
    last_att.gyro_z = gz;
    last_att.valid = true;
    return last_att;
}
