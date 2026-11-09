#include "sensors/bmp388.h"
#include "config.h"

#include <Adafruit_BMP3XX.h>

static Adafruit_BMP3XX bmp;
static bool ok = false;
static float ground_alt_m = 0.0f;
static bool have_ref = false;

void bmp388_init(void) {
    ok = bmp.begin_I2C(BMP388_I2C_ADDR, &Wire);
    if (ok) {
        bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
        bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
        bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
        bmp.setOutputDataRate(BMP3_ODR_50_HZ);
    }
}

void bmp388_set_ground_reference(void) {
    if (ok && bmp.performReading()) {
        ground_alt_m = bmp.readAltitude(1013.25f);
        have_ref = true;
    }
}

baro_state_t bmp388_read(void) {
    baro_state_t s;
    s.pressure_hpa = 0.0f;
    s.temperature_c = 0.0f;
    s.altitude_m = 0.0f;
    s.valid = false;

    if (!ok || !bmp.performReading()) {
        return s;
    }

    s.pressure_hpa = bmp.pressure / 100.0f;
    s.temperature_c = bmp.temperature;
    s.altitude_m = have_ref ? (bmp.readAltitude(1013.25f) - ground_alt_m) : 0.0f;
    s.valid = true;
    return s;
}
