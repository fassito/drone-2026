#pragma once

// ============================================================
// Drone-2026 — Configuración de hardware y constantes
// Placa: ESP32 DevKit V1 (ESP32-WROOM-32, 30 pines)
// ============================================================

#define BOARD_NAME "ESP32 DevKit V1"

// ---------- Puertos serie ----------
#define MONITOR_SPEED 115200

// ---------- Pines ----------
// Motores (OneShot125) — salidas LEDC de alta resolución.
// Se evitan los pines de strapping (GPIO12/MTDI y GPIO15/MTDO) para no
// condicionar el arranque: un nivel erróneo en ellos impide bootear.
#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 26
#define MOTOR_PIN_3 14
#define MOTOR_PIN_4 27

// Receptor FS-IA6B (iBus, UART). GPIO34 es solo de entrada.
#define IBUS_UART_NUM 1
#define IBUS_RX_PIN 34
#define IBUS_TX_PIN (-1)   // no se usa TX

// IMU MPU-9250 (GY-951, SPI). Giroscopio/acelerómetro/magnetómetro + fusión
// Madgwick en el ESP32 a 1 kHz.
#define MPU9250_SPI_SCK  18
#define MPU9250_SPI_MOSI 23
#define MPU9250_SPI_MISO 19
#define MPU9250_SPI_CS   5
#define MPU9250_SPI_HZ   10000000   // 10 MHz
#define MPU9250_USE_MAG  1          // usar magnetómetro AK8963 (9DoF)

// GPS Neo 6M-0-001 (NMEA). El GPS (9600 baud) va por SoftwareSerial.
#define GPS_BAUD 9600
#define GPS_RX_PIN 35     // solo entrada
#define GPS_TX_PIN 4      // requerido por la librería aunque no se use TX

// Radio de telemetría HC-12 (433/868/915 MHz). En UART2 de hardware (pines
// 16/17 libres). 9600 baud. Enlace EN VUELO.
#define HC12_BAUD 9600
#define HC12_UART_NUM 2
#define HC12_RX_PIN 16    // HC-12 TX -> ESP32 RX2
#define HC12_TX_PIN 17    // HC-12 RX <- ESP32 TX2
#define HC12_SET_PIN 13   // HIGH = modo AT, LOW = transparente

// Bus I2C (pantalla OLED y sensores I2C futuros)
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Pantalla OLED I2C (~1 pulgada, SSD1306/SH1106) — comparte el bus I2C.
#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Monitor de batería INA219 (I2C) — comparte el bus I2C con la OLED.
#define INA219_I2C_ADDR 0x40
#define BATTERY_LOW_V     10.5f   // umbral de batería baja (3S) — ajustar al voltaje de la LiPo
#define BATTERY_CELLS    3        // celdas de la LiPo (3S por defecto)

// Barómetro BMP388 (I2C) — comparte el bus I2C con OLED e INA219.
#define BMP388_I2C_ADDR 0x76     // 0x77 si el pin SDO del BMP388 está a VCC
#define BMP388_SDO_HIGH false    // true si SDO está a VCC (cambia la dirección a 0x77)

// Sensores de proximidad (evitar choques): 4x Sharp GP2Y0A21 (IR analógico,
// 10–80 cm) en los canales ADC1. Solo se usan los ADC1 expuestos en el
// DevKit V1 de 30 pines (36, 39, 32, 33): GPIO37/38 no salen al conector.
// ADC2 entra en conflicto con el WiFi activo, por eso no se usan.
#define PROX_SENSOR_COUNT 4
#define PROX_PIN_FRONT  36   // VP
#define PROX_PIN_RIGHT  32
#define PROX_PIN_BACK   33
#define PROX_PIN_LEFT   39   // VN

// LED de estado integrado
#define LED_BUILTIN 2

// Buzzer 5V (avisos de armado/failsafe/batería baja).
// GPIO15 es pin de strapping (MTDO): queda alto al arrancar y la base del
// transistor (1k) no lo fuerza, así que no afecta al boot.
#define BUZZER_PIN 15

// ---------- Seguridad (failsafe) ----------
#define BATTERY_CRITICAL_V 10.0f   // tensión crítica (3S) → corte
#define MAX_CURRENT_A      20.0f   // sobrecorriente → corte

// ---------- Habilitación de periféricos ----------
// Encender cada uno en 1 conforme se conecte (evita leer pines RX flotantes).
// Fase 0 (solo DevKit): todo en 0 salvo WiFi.
#define ENABLE_MPU9250  1    // IMU SPI (Fase 1)
#define ENABLE_I2C      1    // OLED + INA219 + BMP388 (Fase 2)
#define ENABLE_GPS      0    // Neo 6M (Fase 3)
#define ENABLE_HC12     0    // radio telemetría (Fase 4)
#define ENABLE_IBUS     0    // FS-IA6B (Fase 5)
#define ENABLE_SHARP    0    // GP2Y0A21 ADC (Fase 6)

// ---------- Bucle de control ----------
#define LOOP_FREQ_HZ 1000
#define LOOP_PERIOD_US (1000000 / LOOP_FREQ_HZ)
// Núcleo dedicado AL control de vuelo (core 0). Las comunicaciones, sensores
// auxiliares y UI van en el core 1 (donde Arduino ejecuta setup()/loop()).
#define CONTROL_TASK_CORE 0
#define CONTROL_TASK_PRIORITY 10

// ---------- OneShot125 (ESC BLHeli 30A) ----------
#define ONESHOT_MIN_US 125
#define ONESHOT_MAX_US 250
#define ONESHOT_NEUTRAL_US 188
#define ONESHOT_REFRESH_HZ 500   // periodo fijo de 2 ms

// ---------- iBus (FS-IA6B) ----------
#define IBUS_BAUD 115200
#define IBUS_CHANNELS 6

// ---------- Navegación autónoma ----------
#define RTH_ALT_M            5.0f    // altitud de seguridad para RTH
#define GEOFENCE_MAX_DIST_M  200.0f  // radio máximo desde home
#define GEOFENCE_MAX_ALT_M   50.0f   // altitud máxima

// ---------- Black box (registro en vuelo, RAM) ----------
#define BLACKBOX_RATE_HZ      100
#define BLACKBOX_MAX_SAMPLES  1000    // ~10 s a 100 Hz

// ---------- WiFi CLI (solo tierra) ----------
#define WIFI_AP_SSID "Drone-2026"
#define WIFI_AP_PASS "cambia-esta-clave"
#define CLI_TCP_PORT 23
#define OTA_HOSTNAME "drone-2026"
