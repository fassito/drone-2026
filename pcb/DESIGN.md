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

## 7. Estado del esquemático

Generado por script con [`generate_schematic.py`](generate_schematic.py):

- 33 componentes, 28 redes, símbolos incrustados (no depende del buscador).
- **ERC: 0 infracciones**.
- Netlist exportada y verificada contra [`netlist.csv`](netlist.csv).

Regenerar tras cambiar conexiones:

```bash
python3 pcb/generate_schematic.py
flatpak run --command=kicad-cli org.kicad.KiCad sch erc \
    pcb/drone-2026.kicad_sch -o pcb/erc.rpt
```

### Pinout de J1 (VERIFICAR contra la placa física)

| Pin | Señal | Red | Pin | Señal | Red |
|---|---|---|---|---|---|
| 1 | EN | — | 2 | 3V3 | 3V3 |
| 3 | VP / IO36 | SHARP_FR | 4 | GND | GND |
| 5 | VN / IO39 | SHARP_L | 6 | IO15 | — |
| 7 | IO34 | IBUS_RX | 8 | IO2 | LED_PIN |
| 9 | IO35 | GPS_RX | 10 | IO4 | GPS_TX |
| 11 | IO32 | MOTOR3 | 12 | IO16 | HC12_RX |
| 13 | IO33 | MOTOR4 | 14 | IO17 | HC12_TX |
| 15 | IO25 | MOTOR1 | 16 | IO5 | SPI_CS_IMU |
| 17 | IO26 | MOTOR2 | 18 | IO18 | SPI_SCK |
| 19 | IO27 | BUZZER_PIN | 20 | IO19 | SPI_MISO |
| 21 | IO14 | — | 22 | IO21 | I2C_SDA |
| 23 | IO12 | — | 24 | IO22 | I2C_SCL |
| 25 | IO13 | HC12_SET | 26 | IO23 | SPI_MOSI |
| 27 | GND | GND | 28 | GND | GND |
| 29 | VIN | 5V | 30 | 3V3 | 3V3 |

> ⚠️ Este mapeo debe confirmarse con la serigrafía de la placa antes de
> fabricar. Si tu DevKit difiere, edita `ESP32_PINOUT` en
> [`generate_schematic.py`](generate_schematic.py) y regenera.

## 8. Limitación detectada: sensores Sharp R y B

**GPIO37 y GPIO38 no están disponibles** en el DevKit V1 de 30 pines (solo se
exponen VP=IO36 y VN=IO39). Por eso en el esquemático los conectores de los
Sharp **R** (U9) y **B** (U10) quedan sin señal (marcados como no conectados).

Opciones:

1. **Reasignar pines** (recomendado): mover `MOTOR3` de IO32→IO12 y
   `MOTOR4` de IO33→IO14, liberando IO32/IO33 (ADC1) para Sharp R y B.
   Requiere actualizar `firmware/src/config.h`.
2. Usar solo 2 sensores Sharp (delantero y trasero).
3. Cambiar a un módulo de 38 pines.

## 9. Pendiente

1. Layout en el editor de PCB: colocar huellas, enrutar, planos de masa.
2. DRC y exportación de Gerbers.

> La GUI se abre con `flatpak run org.kicad.KiCad`. Las bibliotecas oficiales
> de símbolos y footprints están instaladas como extensión flatpak.
