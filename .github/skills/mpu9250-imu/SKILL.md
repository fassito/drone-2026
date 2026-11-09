---
name: mpu9250-imu
description: 'Integrar la IMU MPU-9250 (GY-951) por SPI en el dron ESP32: leer giroscopio/acelerómetro/magnetómetro, ejecutar fusión Madgwick/Mahony a 1 kHz y calibrar. Use when: conectar el MPU-9250, leer velocidad angular para el lazo de rate, estimar actitud o calibrar el magnetómetro.'
---

# IMU MPU-9250 (GY-951) por SPI

## Cuándo usarlo
- Conectar el MPU-9250 al ESP32 por SPI.
- Leer giroscopio, acelerómetro y magnetómetro a alta tasa.
- Ejecutar la fusión de actitud (Madgwick/Mahony) en el ESP32 a 1 kHz.
- Calibrar giroscopio y magnetómetro.

## Por qué SPI

- El lazo de rate a 1 kHz necesita **velocidad angular del giroscopio** a alta tasa.
- SPI permite leer la IMU mucho más rápido que I2C (10 MHz vs 400 kHz) sin robar
  tiempo al bucle de control.
- La fusión se ejecuta **en el ESP32**, con control total y sin latencia UART.

## Procedimiento

1. **Leer** [mpu9250-spi-wiring.md](./references/mpu9250-spi-wiring.md) para pines, registro y fusión.
2. Cableado SPI (bus dedicado a la IMU, no compartido):
   ```
   MPU-9250 VCC  ──► 3.3 V
   MPU-9250 GND  ──► GND
   MPU-9250 SCL  ──► GPIO18 (SCK)
   MPU-9250 SDA  ──► GPIO23 (MOSI)
   MPU-9250 AD0  ──► GPIO19 (MISO)   ← en SPI, AD0 es el MISO
   MPU-9250 NCS  ──► GPIO5  (CS)
   INT, FSYNC, EDA, ECL  ──► sin conectar (AK8963 interno)
   ```
3. Inicializar SPI a 10 MHz, modo 0, y configurar el MPU-9250:
   - Giroscopio ±2000 dps, acelerómetro ±16 g, DLPF a 92 Hz (gyro) y 41 Hz (acc).
   - Leer registros 0x3B–0x40 (acc) y 0x43–0x48 (gyro) en una ráfaga.
   - Magnetómetro AK8963: configurar lectura continua (0x16) y leer 0x03–0x08.
4. **Calibrar giroscopio**: media de N muestras en reposo → offsets.
5. Ejecutar **Madgwick AHRS** (o Mahony) a 1 kHz con el `dT` real del lazo.
6. Exponer `attitude_t` (yaw/pitch/roll) + velocidad angular (gyro en rad/s).
7. Calibrar magnetómetro (hard/soft iron) y aplicar declinación magnética de la
   zona (ver referencia de calibración).

## Seguridad

- Si la IMU falla o da valores incoherentes, el failsafe de sensores entra en
  modo seguro (no usar datos obsoletos).
- Nunca volar sin calibrar giroscopio y sin haber verificado los signos de los ejes.

## Verificación

- `attitude` en el CLI muestra ángulos estables en reposo (drift bajo).
- Al girar la placa, roll/pitch/yaw responden con los signos correctos.
- El lazo de rate recibe velocidad angular directa, sin derivar actitud.
