---
name: hc12-telemetry
description: 'Integrar el módulo de radio HC-12 (433/868/915 MHz) como enlace de telemetría bidireccional del dron ESP32. Use when: conectar el HC-12 por UART, configurar canal/baud/potencia con comandos AT, enviar telemetría o comandos en vuelo sin interferir con el FS-i6.'
---

# Radio de telemetría HC-12 (enlace en vuelo)

## Cuándo usarlo
- Añadir el enlace bidireccional de telemetría/comandos **en vuelo**.
- Conectar el HC-12 al ESP32 y configurarlo (canal, baud, potencia).
- Reutilizar el catálogo de comandos de la WiFi CLI a través del HC-12.

## Por qué HC-12

- Opera en **433/868/915 MHz**, fuera de la banda de 2.4 GHz de la emisora FS-i6
  → no interfiere con el control de radio.
- Es un **puerto serie transparente**: el ESP32 lo ve como un UART más.
- Alcance ~1000 m en campo abierto (según modo FU y antena).

## Procedimiento

1. **Leer** [hc12-config.md](./references/hc12-config.md) para modos FU, comandos AT y cableado.
2. **Cableado** (módulo HC-12):
   ```
   HC-12 TX  ──► GPIO16 (UART2 RX)
   HC-12 RX  ──► GPIO17 (UART2 TX)
   HC-12 SET ──► GPIO13 (salida; HIGH = modo AT)
   HC-12 VCC ──► 5 V rail (soporta 3.2–5.5 V)
   HC-12 GND ──► GND
   ```
3. Configurar el HC-12 con `AT` (SET en HIGH): canal, baud 9600 8N1 y modo FU
   (FU3 = 9600 baud aire, recomendado para compatibilidad con el firmware).
4. Implementar `telemetry/hc12_link.*` como un **stream** que la CLI consume,
   igual que el socket WiFi: los comandos llegan por el HC-12 y las respuestas
   vuelven por el HC-12.
5. En vuelo, el HC-12 es el enlace activo (el WiFi está apagado al armar).

## Seguridad

- Comandos sensibles (`arm`, `motortest`, `rth`, ...) exigen confirmación, tanto
  por WiFi como por HC-12.
- El HC-12 es un enlace **no cifrado**: no enviar secretos; la autenticación de
  comandos críticos se mantiene en el failsafe (token/número autorizado si se usa).

## Verificación

- Con un HC-12 USB en el PC, enviar `help`/`status` y recibir respuesta.
- Probar el enlace con el WiFi apagado (simulando vuelo).
- Alcance en campo abierto sin pérdida de tramas.
