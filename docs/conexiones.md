# Esquema de conexiones — Drone-2026

Esquema completo de conexionado del dron (ESP32 DevKit V1 + todos los periféricos).

## 1. Distribución de potencia

```
   LiPo 3–4S (XT-60)
        │ BAT+
        ▼
  ┌───────────┐   mide tensión y corriente
  │  INA219   │──► I2C (0x40)
  └─────┬─────┘
        │ BAT+ (pasa a través)
        ▼
                             ┌────────────────────────────────────────────┐
                             │                PDB (XT-60)                 │
                             │  Entrada de batería: XT-60 (LiPo 3–4S)     │
                             └───────┬───────────────┬────────────────────┘
                                     │               │
              +5V/alta corriente     │               │
        ┌────────────┬───────────────┼───────────────┼───────────────┐
        │            │               │               │               │
   ┌────▼───┐   ┌────▼───┐     ┌─────▼────┐    ┌─────▼────┐    ┌─────▼────┐
   │ ESC 1  │   │ ESC 2  │     │ ESC 3    │    │ ESC 4    │    │  BEC 5V  │
   │ (FR)   │   │ (RL)   │     │ (FL)     │    │ (RR)     │    │  5V/3A   │
   └────┬───┘   └────┬───┘     └─────┬────┘    └─────┬────┘    └─────┬────┘
        │            │               │               │               │
        ▼            ▼               ▼               ▼               │ 5V regulado
     Motor 1      Motor 2         Motor 3         Motor 4            ▼
     (FR,CW)      (RL,CW)         (FL,CCW)        (RR,CCW)    ┌──────────────┐
                                                              │ 5V RAIL      │
                                                              └──────┬───────┘
                                                                     │
   ┌──────────────┬───────────────────────┬───────────────────┬──────┘
   │              │                       │                   │
   ▼              ▼                       ▼                   ▼
ESP32 VIN    FS-IA6B (5V)         Sharp x4 (VCC 5V)      Buzzer 5V
(regulador   (receptor)            (IR analógico)         (opcional)
 interno 3.3V)
        │
        └──► AMS1117 (integrado) ──► 3.3V RAIL
                                       ├──► OLED (0x3C)
                                       ├──► MPU-9250
                                       ├──► BMP388
                                       └──► INA219 VCC
```

### Notas de alimentación

| Componente | Tensión | Fuente |
|---|---|---|
| ESP32 (VIN) | 5 V | BEC 5 V → regulador AMS1117 integrado en la placa (5 V→3.3 V) |
| ESC 1–4 | Tensión de batería | PDB (pads de batería) |
| FS-IA6B | 4.0–6.5 V | BEC 5 V |
| MPU-9250 (IMU) | 2.4–3.6 V | ESP32 3.3 V |
| GPS Neo 6M | 3.3–5 V | 5 V rail (o 3.3 V) |
| Sharp GP2Y0A21 ×4 | 4.5–5.5 V | 5 V rail |
| OLED | 3.3 V (o 5 V según módulo) | ESP32 3.3 V / 5 V rail |
| INA219 (batería) | 3–5 V | 5 V rail (mide el bus de batería) |
| BMP388 (barómetro) | 1.71–3.6 V | ESP32 3.3 V |
| HC-12 (telemetría) | 3.2–5.5 V | 5 V rail (~100 mA TX) |

> ⚠️ Los ESC BLHeli 30A suelen ser **OPTO** (sin BEC): toda la electrónica de
> control se alimenta desde el BEC de 5 V, no desde los ESC.

### Qué resuelve (y qué NO) la placa DevKit V1

La placa de desarrollo **sí** aporta:

- **Regulador AMS1117** integrado: convierte los 5 V de `VIN` a 3.3 V para el
  ESP32 y los periféricos de 3.3 V (OLED, MPU-9250, BMP388, GPS si es 3.3 V).
- **USB-serial** (CH340/CP2102) para grabar y depurar por UART0.

La placa **NO** resuelve:

- **Tolerancia a 5 V en los GPIO**: los pines del ESP32 siguen siendo 3.3 V y no
  toleran 5 V. El iBus del FS-IA6B (5 V) **requiere divisor/nivelador** en GPIO34.
- **ADC2 vs WiFi**, el **número de UART** o la contención de SoftwareSerial:
  son limitaciones del chip, no de la placa.

> Presupuesto de la rama de 3.3 V (AMS1117): reservar el 3.3 V para carga ligera
> (OLED ~20 mA, GPS ~50 mA, MPU-9250 ~3.5 mA, INA219 ~1 mA). Los Sharp, el buzzer
> y el receptor van a la rama de 5 V.

---

## 2. Señales de datos (ESP32)

```
                            ┌───────────────────────────────────────────┐
                            │            ESP32 DevKit V1               │
                            │                                           │
  MPU-9250 ──(SPI)─────────►│ SCK=18(SCL) MOSI=23(SDA) MISO=19(AD0) CS=5(NCS) │
  FS-IA6B   ──(iBus)───────►│ RX1 = GPIO34                              │
  GPS       ──(SoftSerial)─►│ RX  = GPIO35   TX  = GPIO4  ──► GPS       │
  HC-12     ──(UART2)──────►│ RX2 = GPIO16   TX2 = GPIO17 ──► HC-12     │
                            │        SET = GPIO13 (modo AT)             │
                            │                                           │
  OLED ──────(I2C)──────────│ SDA = GPIO21   SCL = GPIO22               │
  INA219 ────(I2C)──────────│ (comparte bus I2C con la OLED, 0x40)      │
  BMP388 ────(I2C)──────────│ (comparte bus I2C, 0x76/0x77)             │
                            │                                           │
  Sharp FR  ──(ADC)────────►│ GPIO36                                   │
  Sharp R   ──(ADC)────────►│ GPIO37                                   │
  Sharp B   ──(ADC)────────►│ GPIO38                                   │
  Sharp L   ──(ADC)────────►│ GPIO39                                   │
                            │                                           │
  ESC 1 ───(OneShot125)────►│ GPIO25                                   │
  ESC 2 ───(OneShot125)────►│ GPIO26                                   │
  ESC 3 ───(OneShot125)────►│ GPIO14                                   │
  ESC 4 ───(OneShot125)────►│ GPIO27                                   │
                            │                                           │
  Buzzer 5V ────────────────│ GPIO15                                   │
  LED estado ───────────────│ GPIO2                                    │
                            └───────────────────────────────────────────┘
```

---

## 3. Tabla completa de pines

### Comunicaciones serie

| Periférico | Interfaz | Pin ESP32 | Notas |
|---|---|---|---|
| FS-IA6B (iBus) | UART1 RX | GPIO 34 | Solo entrada (no TX). 115200 baud. **Obligatorio divisor/nivelador 5 V→3.3 V** (p. ej. 2.2 kΩ serie + 4.7 kΩ a GND) |
| MPU-9250 (IMU) | SPI | SCK 18 / MOSI 23 / MISO 19 / CS 5 | 10 MHz. Pines de la placa: SCL→18, SDA→23, AD0→19, NCS→5 |
| GPS Neo 6M | SoftwareSerial RX/TX | GPIO 35 / 4 | 9600 baud |
| HC-12 (telemetría) | UART2 RX/TX | GPIO 16 / 17 | 9600 baud; SET = GPIO 13 |

### I2C

| Periférico | Pin ESP32 | Notas |
|---|---|---|
| OLED SDA | GPIO 21 | Dirección 0x3C (SSD1306/SH1106) |
| OLED SCL | GPIO 22 | Bus compartido |
| INA219 (batería) | SDA 21 / SCL 22 | Dirección 0x40; tensión + corriente |
| BMP388 (barómetro) | SDA 21 / SCL 22 | Dirección 0x76 (o 0x77 si SDO a VCC) |

> **Módulo CJMCU-388** (pines Vin/3Vo/Gnd/SCK/SDO/SDI/CS/INT) en modo **I2C**:
>
> | Pin módulo | Conexión |
> |---|---|
> | Vin | ESP32 3.3 V |
> | Gnd | ESP32 GND |
> | SCK | GPIO 22 (SCL) |
> | SDI | GPIO 21 (SDA) |
> | **CS** | **3.3 V** (obligatorio a alto para modo I2C) |
> | **SDO** | **GND** → dirección 0x76 (a 3.3 V → 0x77) |
> | 3Vo | sin conectar (salida regulada) |
> | INT | sin conectar (interrupción no usada) |

### Salidas de motores (OneShot125)

| Motor | Pin ESP32 | Posición | Giro |
|---|---|---|---|
| ESC 1 | GPIO 25 | Delante-derecha (FR) | CW |
| ESC 2 | GPIO 26 | Detrás-izquierda (RL) | CW |
| ESC 3 | GPIO 14 | Delante-izquierda (FL) | CCW |
| ESC 4 | GPIO 27 | Detrás-derecha (RR) | CCW |

> GPIO32/33 se liberan para los sensores Sharp (ADC1). Se evitan GPIO12 y
> GPIO15 como salidas de motor: son pines de *strapping* y un nivel erróneo
> al arrancar impide el boot.

### Entradas analógicas (Sharp GP2Y0A21, ADC1)

| Sensor | Pin ESP32 | Dirección |
|---|---|---|
| Sharp FR | GPIO 36 (VP) | Frente |
| Sharp R | GPIO 32 | Derecha |
| Sharp B | GPIO 33 | Atrás |
| Sharp L | GPIO 39 (VN) | Izquierda |

### Salidas auxiliares

| Periférico | Pin ESP32 | Notas |
|---|---|---|
| Buzzer 5 V | GPIO 15 | Avisos de armado/failsafe/batería |
| LED estado | GPIO 2 | Integrado en placa (strapping: mantener alto al arrancar) |

> Solo se usan los **ADC1 expuestos en el DevKit V1 de 30 pines**: 36 (VP),
> 39 (VN), 32 y 33. **GPIO37 y GPIO38 no salen al conector** en esta placa.
> ADC2 queda descartado porque entra en conflicto con el WiFi activo.

---

## 4. Alimentación de cada módulo (resumen visual)

```
BATERÍA LiPo 3–4S (XT-60)
        │
        ▼
      PDB (XT-60)
        ├──► ESC 1  (pad +) ──► Motor 1
        ├──► ESC 2  (pad +) ──► Motor 2
        ├──► ESC 3  (pad +) ──► Motor 3
        ├──► ESC 4  (pad +) ──► Motor 4
        └──► BEC 5V/3A
               ├──► ESP32 VIN
               ├──► FS-IA6B (5V)
               ├──► Sharp ×4 (VCC 5V)
               ├──► Buzzer 5V (opcional)
               └──► GPS Neo 6M (VCC, si acepta 5V)

     MPU-9250     ◄── 3.3V del ESP32
     OLED         ◄── 3.3V del ESP32 (o 5V según módulo)
     BMP388       ◄── 3.3V del ESP32
     HC-12        ◄── 5V rail (~100 mA TX)
     GND común: todos los GND unidos al GND del PDB.
```

---

## 5. Divisores de tensión y pull-ups (detalle)

### 5.1 Divisor iBus 5 V → 3.3 V (obligatorio en GPIO34)

```
FS-IA6B iBus (5V) ──► R1 = 2.2 kΩ ──┬──► GPIO34 (Vout ≈ 3.4 V)
                                     │
                                     ├──► R2 = 4.7 kΩ
                                     │
                                    GND

Vout = 5 V × 4.7 / (2.2 + 4.7) ≈ 3.4 V   ✓ dentro del rango 3.3 V
```

### 5.2 Divisor GPS 5 V → 3.3 V (solo si el módulo Neo 6M es de 5 V)

```
GPS TX (5V) ──► R1 = 2.2 kΩ ──┬──► GPIO35
                               │
                               ├──► R2 = 4.7 kΩ
                               │
                              GND
```

> Si el módulo GPS es de **3.3 V**, conectar TX directo a GPIO35 (sin divisor).

### 5.3 Pull-ups del bus I2C (SDA + SCL)

```
3.3V ──► R = 4.7 kΩ ──┬──► SDA (GPIO21)
                      │
3.3V ──► R = 4.7 kΩ ──┼──► SCL (GPIO22)
                      │
        OLED (0x3C) ──┴── INA219 (0x40) ── BMP388 (0x76)
```

- La mayoría de módulos OLED ya traen pull-ups soldados; si no, añadir 4.7 kΩ
  entre cada línea (SDA/SCL) y 3.3 V.

---

## 6. Referencias cruzadas

- Pines y constantes en [config.h](../firmware/src/config.h).
- Alimentación de ESC y PDB: skill [esc-setup](../.github/skills/esc-setup/SKILL.md).
- Cableado iBus y binding: [ibus-protocol.md](../.github/skills/ibus-integration/references/ibus-protocol.md).
- MPU-9250: [mpu9250-spi-wiring.md](../.github/skills/mpu9250-imu/references/mpu9250-spi-wiring.md).
- GPS: [nmea-protocol.md](../.github/skills/gps-nav/references/nmea-protocol.md).
- HC-12: [hc12-config.md](../.github/skills/hc12-telemetry/references/hc12-config.md).
- Sharp GP2Y0A21: [sensor-options.md](../.github/skills/obstacle-avoidance/references/sensor-options.md).
- OLED: [ssd1306-vs-sh1106.md](../.github/skills/oled-display/references/ssd1306-vs-sh1106.md).

---

## 7. Estrategia de operación (WiFi híbrido + batería)

### WiFi solo en tierra

- El WiFi es un canal de **diagnóstico en tierra**; NO es un enlace de vuelo.
- **Al armar** → el AP WiFi se **desactiva automáticamente** para no interferir
  con la FS-i6 (ambas en 2.4 GHz) y para liberar ADC2.
- **Al desarmar** → se reactiva (o manualmente con `wifi on`).
- En vuelo el control depende solo del enlace FS-i6 y de los modos autónomos
  (RTH, misión). La telemetría en vuelo se ve en la OLED y por el **HC-12**.

### Enlace bidireccional en vuelo (HC-12)

- El **HC-12** (433/868/915 MHz) es el enlace de telemetría/comandos **en vuelo**,
  fuera de la banda 2.4 GHz del mando FS-i6.
- Es un stream serie transparente: reutiliza el mismo parser de comandos de la
  WiFi CLI (leer/escribir por el HC-12).
- Conexión: TX→GPIO16, RX→GPIO17, SET→GPIO13, VCC→5 V. Ver skill
  [hc12-telemetry](../.github/skills/hc12-telemetry/SKILL.md).

### Monitor de batería (INA219, I2C)

- Mide **tensión** y **corriente** de la batería en el bus I2C (dirección 0x40),
  compartido con la OLED.
- Se elige INA219 (no divisor ADC) porque los 4 ADC1 están ocupados por los Sharp
  y ADC2 queda bloqueado con el WiFi activo.
- La lectura es I2C (lenta): se hace a baja frecuencia en una tarea separada; el
  failsafe consume el último valor válido.
- Umbral de batería baja en `config.h` (`BATTERY_LOW_V`), según la LiPo usada.
