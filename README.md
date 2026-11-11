# Drone-2026

Controladora de vuelo para cuadricóptero basada en **ESP32 (DevKit V1)**,
programada con el **framework Arduino** bajo **PlatformIO**.

## Objetivos del proyecto

1. **Controladora de vuelo en ESP32** con bucle de estabilización a 1 kHz:
   PID en cascada (lazo externo de ángulo + lazo interno de velocidad angular).
2. **Accionamiento de 4 motores brushless** mediante ESC programables
   **BLHeli 30A** con protocolo **OneShot125** (3–6S LiPo).
3. **Doble enlace de comunicación**:
   - **RF** con emisora **FlySky FS-i6** + receptor **FS-IA6B** (protocolo iBus por UART).
   - **WiFi** con interfaz de línea de comandos (CLI) para diagnóstico,
     pruebas y telemetría en tiempo real.
4. **Autoajuste de parámetros PID** para la estabilización.
5. **Seguridad como requisito de primer orden**: armado explícito,
   failsafe de radio y WiFi, corte de motores y procedimientos de prueba seguros.

## Lista de materiales (BOM)

| Componente | Referencia | Notas |
|---|---|---|
| Microcontrolador | ESP32 DevKit V1 (ESP32-WROOM-32, 30 pines) | 240 MHz, dual-core, WiFi |
| Motores | 4× Crozepony DX2205 | 2300 KV, brushless |
| ESC | 4× BLHeli 30A | OneShot125, 3–6S |
| PDB | Power Distribution Board con XT-60 | Reparte la batería a los 4 ESC y al BEC |
| Receptor | FlySky FS-IA6B | 6 canales, iBus UART 115200 |
| Emisora | FlySky FS-i6 | 6 canales, AFHDS 2A, compatible con iBus |
| Batería | LiPo **4S 1550 mAh** (14.8 V) | Alimenta ESCs; BEC/regulador para el ESP32 |
| IMU | MPU-9250 (GY-951) | 9DoF por SPI; fusión Madgwick en el ESP32 a 1 kHz |
| GPS | u-blox NEO-6M-0-001 | NMEA por UART 9600; navegación, waypoints y RTH |
| Monitor de batería | INA219 (I2C) | Monitor de tensión y corriente; alimenta el failsafe de batería baja |
| Barómetro | BMP388 (I2C) | Altitud barométrica para mantener altura con precisión |
| Telemetría en vuelo | HC-12 (433/868/915 MHz) | Enlace bidireccional de comandos/telemetría, fuera de la banda 2.4 GHz |
| Pantalla | OLED I2C ~1" (SSD1306/SH1106) | Telemetría y diagnóstico en pantalla |
| Proximidad | 4× Sharp GP2Y0A21 (IR analógico) | Evitación de choques; salida ADC (ADC1), 10–80 cm |
| Frame + hélices | Cuadricóptero X | Hélice acorde a motor/frame |

## Pendiente de comprar / definir

| Componente | Detalle | Estado |
|---|---|---|
| Frame | Cuadricóptero X ~210–250 mm (fibra de carbono) | 🔴 pendiente |
| Hélices | 5" (p. ej. 5045 tri-blade), 2 CW + 2 CCW + repuestos | 🔴 pendiente |
| BEC 5 V | 5 V / 3 A (los ESC BLHeli 30A son OPTO, sin BEC) | 🔴 pendiente |
| LiPo | **4S 1550 mAh 100C** con XT-60 (DX2205 2300 KV) | 🔴 pendiente |
| Cargador LiPo | Balanceado (IMAX B6 o similar) + bolsa ignífuga | 🔴 pendiente |
| **Nivelador iBus** | Divisor resistivo 5 V→3.3 V (2.2 kΩ + 4.7 kΩ) o módulo nivelador para GPIO34 | 🟡 **obligatorio** |
| Soporte antivibración | Goma/espuma para la IMU MPU-9250 | 🟡 obligatorio |
| Buzzer 5 V | Avisos de armado/failsafe/batería baja (GPIO 15) | 🟡 implementado en código, comprar pieza |
| Monitor de batería | INA219 (ya en BOM) | ✅ decidido |
| Barómetro | BMP388 (ya en BOM) | ✅ decidido |
| Telemetría en vuelo | HC-12 (ya en BOM) | ✅ decidido |

## Arquitectura de subsistemas

```
firmware/
├── src/
│   ├── main.cpp              # Arranque y tareas
│   ├── config.h              # Pines, constantes y parámetros por defecto
│   ├── flight/
│   │   ├── mixer.*           # Mezcla de motores (quad X)
│   │   └── pid.*             # PID en cascada
│   ├── sensors/
│   │   ├── mpu9250.*         # IMU MPU-9250 (SPI) + fusión Madgwick
│   │   └── bmp388.*          # Barómetro BMP388 (altitud barométrica)
│   ├── actuators/
│   │   └── esc.*             # Señal OneShot125
│   ├── radio/
│   │   └── ibus.*            # Decodificación iBus FS-IA6B
│   ├── telemetry/
│   │   ├── wifi_cli.*        # CLI por WiFi (TCP, solo en tierra)
│   │   ├── hc12_link.*       # Radio HC-12 (telemetría/comandos en vuelo)
│   │   └── oled_display.*    # Pantalla OLED I2C (estado y diagnóstico)
│   ├── nav/
│   │   ├── gps.*             # GPS Neo 6M (NMEA), waypoints y RTH
│   │   ├── obstacle.*        # Sensores de proximidad y evitación de choques
│   │   └── mission.*         # Misión autónoma a destino
│   └── safety/
│       ├── failsafe.*        # Armado, failsafe y corte de motores
│       └── power_monitor.*   # Monitor de batería (INA219, I2C)
└── platformio.ini
```

## Personalizaciones incluidas

- **Directrices del proyecto**: [copilot-instructions.md](.github/copilot-instructions.md)
- **Esquema de conexiones (ASCII)**: [docs/conexiones.md](docs/conexiones.md)
- **Agentes especializados**: [.github/agents/](.github/agents/)
- **Skills (flujos de trabajo)**: [.github/skills/](.github/skills/)

## Comandos rápidos

```bash
cd firmware
pio run                        # compilar firmware (ESP32)
pio run -t upload              # grabar firmware
pio device monitor -b 115200   # monitor serie
./run_host_tests.sh            # batería de tests de host (sin hardware)
```

En el dispositivo (con componentes), ejecutar `selftest` desde la CLI (WiFi o HC-12)
para comprobar I2C, IMU, barómetro, batería, receptor, GPS y sensores de proximidad.

## Estado del firmware

El firmware ya está **implementado y compila** (`pio run` → SUCCESS):

| Módulo | Estado |
|---|---|
| `flight/` — PID cascada + mezcla quad X + autoajuste (relé/ZN) | ✅ implementado |
| `nav/` — GPS, obstáculos, misión multi-waypoint, RTH seguro, geofence, altitud, piloto automático | ✅ implementado |
| `sensors/mpu9250.*` — MPU-9250 SPI + Madgwick **9DoF** + calibración mag | ✅ implementado |
| `sensors/bmp388.*` — barómetro I2C | ✅ implementado |
| `safety/` — failsafe (batería/sobrecorriente), INA219, indicadores (buzzer), black box, autotest | ✅ implementado |
| `radio/ibus.*` — FS-IA6B (UART1) | ✅ implementado |
| `actuators/esc.*` — OneShot125 (LEDC) | ✅ implementado |
| `telemetry/` — WiFi CLI, HC-12, OLED, telemetría JSON, OTA | ✅ implementado |
| `main.cpp` — tareas dual-core | ✅ integrado |

**Tests**: batería en host con **136 checks** por componente e integración, más
autotest en dispositivo (`selftest`).

> Pendiente de hardware: calibración de la IMU y del magnetómetro, signos de los
> ejes y ganancias PID definitivas (ver skills `mpu9250-imu` y `pid-autotune`).

## Montaje y test por fases

Los periféricos se activan por flags `ENABLE_*` en [config.h](firmware/src/config.h)
(arrancan en 0 para no leer pines RX flotantes). Enciende cada uno en 1 conforme
lo conectes y valida con `selftest`:

| Fase | Componente | Flag a activar | Verificación |
|---|---|---|---|
| 0 | Solo DevKit | — | monitor serie: `[ctrl] 1000Hz ...` |
| 1 | MPU-9250 (SPI) | `ENABLE_MPU9250` | `selftest` → WHO_AM_I=0x71 |
| 2 | OLED + INA219 + BMP388 | `ENABLE_I2C` | `selftest` + `baro`/`power` |
| 3 | GPS Neo 6M | `ENABLE_GPS` | `gps` → fix=1 al aire libre |
| 4 | HC-12 | `ENABLE_HC12` | `status` por radio desde el PC |
| 5 | FS-IA6B | `ENABLE_IBUS` | `selftest` → enlace activo |
| 6 | Sharp GP2Y0A21 | `ENABLE_SHARP` | `obstacle` → distancias |
| 7 | Potencia + motores | — | `motortest` **sin hélices** |


> ⚠️ **Seguridad**: nunca realizar pruebas con hélices montadas. Toda prueba
> de motores debe hacerse con el armado bloqueado y el failsafe activo.
