# Referencia HC-12

## Características

| Dato | Valor |
|---|---|
| Banda | 433.4–473.0 MHz (existen versiones 868/915 MHz) |
| Interfaz | UART serie transparente (TTL 3.3 V) |
| Tensión | 3.2–5.5 V (alimentar a 5 V para mayor potencia) |
| Corriente TX | ~100 mA a máxima potencia |
| Alcance | ~1000 m en campo abierto (modo FU3) |
| Pin SET | HIGH = modo comandos AT, LOW = transparente |

## Cableado (ESP32)

| HC-12 | ESP32 |
|---|---|
| TX | GPIO 16 (UART2 RX) |
| RX | GPIO 17 (UART2 TX) |
| SET | GPIO 13 (salida) |
| VCC | 5 V rail |
| GND | GND |

> El HC-12 es lógica 3.3 V: conexión directa a los GPIO del ESP32.
> Va en el UART2 de hardware (pines 16/17 libres).

## Modos FU (potencia y baud en el aire)

| Modo | Baud aire | Potencia | Notas |
|---|---|---|---|
| FU1 | 9600 | ~-8 dBm | Bajo consumo |
| FU2 | 9600 | ~+3 dBm | |
| **FU3** | **9600** | **~+10 dBm** | **Recomendado** |
| FU4 | 9600 | ~+20 dBm | Máxima potencia (más consumo) |

## Comandos AT (con SET en HIGH)

| Comando | Uso |
|---|---|
| `AT` | Test (responde `OK`) |
| `AT+B9600` | Baud serie 9600 (por defecto) |
| `AT+C001` | Canal 1 (001–127) |
| `AT+FU3` | Modo FU3 |
| `AT+P8` | Potencia de emisión (1–8) |
| `AT+V` | Versión |

## Integración con la CLI

- El HC-12 es transparente: lo que el ESP32 escribe por él llega al HC-12 USB del PC.
- Reutilizar el parser de la WiFi CLI: leer comandos del stream HC-12 y responder
  por el mismo stream.
- En vuelo (WiFi apagado) el HC-12 es el único enlace de telemetría.

## Nota sobre UART

- El HC-12 va en el **UART2 de hardware** (GPIO16/17). El GPS sigue en
  SoftwareSerial a 9600 baud, con baja tasa y sin degradar el lazo de 1 kHz.
