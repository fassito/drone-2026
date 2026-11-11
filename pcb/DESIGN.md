# Placa de integración Drone-2026 — Especificación de diseño

Placa *carrier* que consolida todo el cableado del dron (ver
[`docs/conexiones.md`](../docs/conexiones.md) y
[`firmware/src/config.h`](../firmware/src/config.h)). El ESP32 DevKit V1 va
enchufable: se conservan el USB, el regulador AMS1117 y los botones.

## 1. Objetivo

Reducir el cableado y los errores de montaje, manteniendo los módulos ya
validados (GY-951, CJMCU-388, OLED, GPS, HC-12, Sharp, INA219). **No** es una
placa de control integrada con ESP32 desnudo (esa sería la v2).

## 2. Decisiones de diseño (firmes)

| Tema | Decisión | Motivo |
|---|---|---|
| Alimentación de motores | ESC directos a `VBAT` | la corriente de motor NO pasa por el shunt del INA219 |
| Medición de batería | INA219 mide `VBAT` (tensión) y la corriente de la rama BEC | failsafe de batería baja + voltaje en OLED |
| Sobrecorriente de motores | Sin sensor hall (no comprar componentes) | se cubre con failsafe de caída de tensión + límite de inclinación |
| iBus (5 V) | Divisor 2.2 kΩ serie + 4.7 kΩ a GND | GPIO34 es 3.3 V, no tolera 5 V |
| Buzzer 5 V | Transistor NPN 2N2222 + diodo flyback 1N4148 | el GPIO27 (3.3 V) no puede alimentar el buzzer directamente |
| Capas | 2 capas | suficiente para 100 A de pico en pistas de potencia con refuerzo |
| Conectores | JST-XH para señales; XT-60 para batería | estándar y con enclavamiento |

## 3. Topología de potencia

```
XT-60 (VBAT) ──┬──► ESC1..ESC4  (V+ directo)
               │
               ├──► INA219 Vin+ ──► Vin- ──► BEC 5V IN+ ──► 5V rail
               │                              (corriente electrónica)
               └──► GND común
```

- **VBAT**: pista/zona de cobre ancha (≥ 8 mm) o plano dedicado.
- **GND**: plano de masa en la cara inferior.
- **5V**: rama del BEC → buzzer, HC-12, Sharp, iBus, FS-IA6B.
- **3V3**: del regulador del DevKit (VIN=5V) → OLED, BMP388, INA219, GY-951, GPS.

## 4. Redes (30) — resumen

Ver [`netlist.csv`](netlist.csv) (104 conexiones) y [`BOM.csv`](BOM.csv).

| Red | Pines | Categoría |
|---|---|---|
| VBAT | XT-60+, INA219 Vin+, ESC1-4 V+ | Potencia |
| 5V | BEC out+, HC-12, Sharp×4, iBus, buzzer+ | Potencia |
| 3V3 | DevKit 3V3, OLED, BMP, INA219, GY-951, GPS | Potencia |
| GND | 23 pines | Retorno |
| I2C_SDA / I2C_SCL | OLED, BMP, INA219, DevKit 21/22 | Bus |
| SPI_* | GY-951 ↔ DevKit 5/18/19/23 | Bus |
| MOTOR1..4 | DevKit 25/26/32/33 → ESC | Señal |
| SHARP_* | Sharp×4 → DevKit 36/37/38/39 | ADC1 |
| IBUS_RAW / IBUS_RX | FS-IA6B → divisor → GPIO34 | Señal |
| GPS_RX / GPS_TX | GPS ↔ DevKit 35/4 | UART |
| HC12_* | HC-12 ↔ DevKit 16/17/13 | UART2 |
| BUZZER_* / LED_PIN | driver buzzer / LED | Señal |

## 5. Reglas de layout

- **Pista de señal**: 0.25 mm, clearance 0.2 mm.
- **Pista de potencia**: VBAT/5V/GND ≥ 1.5 mm (ideal 2 mm), reforzar con zona.
- **Pull-ups I2C**: 4.7 kΩ en SDA/SCL (el firmware activa los internos del
  ESP32; los externos se recomiendan en la placa).
- **Condensadores**: 100 nF cerámico en VCC de cada módulo (GY-951, BMP, GPS,
  HC-12, Sharp), 100 µF electrolítico en la entrada del BEC y en 5V.
- **Agujeros de montaje**: 4× M3 en patrón 30.5×30.5 mm (estándar quads 5").
- **Antenas**: HC-12 y GPS con cobre despejado alrededor (sin plano debajo).

## 6. Montaje / fabricación

- Dimensiones objetivo: **50×50 mm** (ajustable al frame final).
- Fabricante: JLCPCB / PCBWay, 2 capas, FR-4 1.6 mm, cobre 1 oz (2 oz en VBAT).
- Conector DevKit: tiras de pines hembra 2.54 mm (30 pines, solo se usan 25).

## 7. Pendiente (requiere GUI de KiCad)

1. Esquemático: importar [`netlist.csv`](netlist.csv) y colocar símbolos.
2. Layout: footprints, enrutado, planos de masa.
3. ERC + DRC, exportar Gerbers.

> La GUI se abre con `flatpak run org.kicad.KiCad`. Las bibliotecas oficiales
> de símbolos y footprints se instalan como extensión flatpak.
