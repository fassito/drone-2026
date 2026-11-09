# MPU-9250 por SPI — cableado, registros y fusión

## Identificación

- **GY-951**: placa con **MPU-9250** (9DoF: giroscopio + acelerómetro MPU-6500 y
  magnetómetro AK8963).
- El MPU-9250 es el reemplazo moderno del MPU-9150 (descatalogado).
- Interfaz: **SPI** (recomendado) o I2C. Dirección I2C 0x68 (AD0 a GND) / 0x69 (AD0 a VCC).

## Cableado SPI (ESP32)

> Variante de 10 pines: SCL, SDA, EDA, ECL, AD0, INT, NCS, FSYNC (+ VCC, GND).

| Pin MPU-9250 | ESP32 | Función | Conectar |
|---|---|---|---|
| VCC | 3.3 V | alimentación | ✅ |
| GND | GND | masa | ✅ |
| SCL | GPIO 18 | SCK | ✅ |
| SDA | GPIO 23 | MOSI (SDI) | ✅ |
| AD0 | GPIO 19 | MISO (SDO) en SPI | ✅ |
| NCS | GPIO 5 | CS | ✅ |
| INT | — | interrupción | ⛔ sin conectar |
| FSYNC | — | frame sync | ⛔ sin conectar |
| EDA | — | bus aux AK8963 (interno) | ⛔ sin conectar |
| ECL | — | bus aux AK8963 (interno) | ⛔ sin conectar |

> En esta placa, `AD0` en modo SPI es el **MISO** (a veces rotulado "SDO").
> En modo I2C, `AD0` elegiría la dirección (GND=0x68, VCC=0x69), pero no aplica en SPI.
> `EDA`/`ECL` son el bus auxiliar del magnetómetro AK8963, ya cableado internamente:
> se lee por el I2C-master del MPU-9250 vía SPI (no se conecta nada externo).

## Configuración de registros

- PWR_MGMT_1 (0x6B): salir de reset (bit 6), reloj interno.
- GYRO_CONFIG (0x1B): FS_SEL=3 → ±2000 dps.
- ACCEL_CONFIG (0x1C): AFS_SEL=3 → ±16 g.
- CONFIG (0x1A): DLPF → ancho de banda de giroscopio 92 Hz (o 184 Hz).
- ACCEL_CONFIG2 (0x1D): DLPF del acelerómetro.
- INT_PIN_CFG (0x37): bit 1 (BYPASS_EN) para acceso directo al AK8963.

### Lectura en ráfaga

- Acelerómetro: 0x3B → 6 registros (X,Y,Z, int16).
- Giroscopio: 0x43 → 6 registros (X,Y,Z, int16).
- Magnetómetro AK8963: iniciar medida continua escribiendo 0x16 en 0x0A, leer 0x03 → 6 registros.

## Escalado

- Giroscopio ±2000 dps: `rad/s = raw / 16.4 * DEG_TO_RAD`.
- Acelerómetro ±16 g: `m/s² = raw / 2048 * 9.81`.
- Magnetómetro: escala según el AK8963 (aprox. 0.6 µT/LSB en 16 bits).

## Fusión de actitud (en el ESP32)

- Ejecutar **Madgwick AHRS** (recomendado) o **Mahony** a 1 kHz con el `dT` real.
- El giroscopio domina a corto plazo; acelerómetro y magnetómetro corrigen el drift
  a largo plazo.
- La **velocidad angular sale directa del giroscopio** para el lazo de rate:
  sin derivar actitud (menos ruido y retardo).

## Calibración

1. **Giroscopio**: medir el offset en reposo (media de ~1000 muestras) y restarlo.
2. **Acelerómetro**: opcional calibración de 6 posiciones (1 g por eje).
3. **Magnetómetro** (hard/soft iron): girar la placa en los 3 ejes y ajustar
   elipsoide a esfera. Aplicar **declinación magnética** de la zona de vuelo.

## Declinación magnética — Madrid (España)

- Zona de vuelo: Madrid (40.42° N, 3.70° W, ~650 m).
- Declinación: **≈ +0.5° Este** (2025–2026, modelo IGRF; deriva ~+0.14°/año).
- En la fusión: `yaw_corregido = yaw_magnetico + 0.5°` (positivo = Este).
- Verificar el signo apuntando el eje al norte geográfico real.
