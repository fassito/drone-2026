---
name: oled-display
description: 'Integrar una pantalla OLED I2C (~1 pulgada, GND/VCC/SCL/SDA) en el dron ESP32: SSD1306 0.96" o SH1106 1.3". Use when: conectar la pantallita OLED por I2C, mostrar telemetría, estado de armado, GPS o diagnóstico en pantalla.'
---

# Pantalla OLED I2C (~1 pulgada)

## Cuándo usarlo
- Conectar una pantalla OLED con solo GND/VCC/SCL/SDA al ESP32.
- Mostrar telemetría esencial: estado de armado, actitud, GPS, obstáculos, IP WiFi.
- Mostrar menús o avisos de diagnóstico sin ordenador.

## Procedimiento

1. **Identificar el controlador** (ambos comparten los 4 pines GND/VCC/SCL/SDA):
   - **SSD1306** 0.96" 128×64 (el más común) → dirección I2C `0x3C`.
   - **SH1106** 1.3" 128×64 → dirección I2C `0x3C` (a veces `0x3D`).
   - Verificar con un escáner I2C si no está seguro.
2. **Cableado** (bus I2C ya definido en `config.h`):
   ```
   OLED GND ──► GND
   OLED VCC ──► 3.3 V (o 5 V según módulo)
   OLED SCL ──► GPIO 22 (I2C SCL)
   OLED SDA ──► GPIO 21 (I2C SDA)
   ```
   Nota: los Sharp usan ADC y no compiten por el I2C, así que el bus queda libre
   para la pantalla.
3. Añadir la librería en `platformio.ini`:
   - SSD1306: `adafruit/Adafruit SSD1306 @ ^2.5.7` (+ `adafruit/Adafruit GFX Library`).
   - SH1106: `adafruit/Adafruit SH110X` (+ GFX) u `olikraus/U8g2` (ambos chips).
4. Implementar `telemetry/oled_display.*`:
   - `oled_init()`: inicializa I2C y la pantalla; limpia el buffer.
   - `oled_render()`: redibuja una vista (no bloquear el lazo de control; refresco
     lento, p. ej. 5–10 Hz en tarea separada).
   - Vistas: `STATUS` (armado/modo), `ATTITUDE`, `GPS`, `OBSTACLE`.
5. Añadir comando CLI para alternar vistas: `display <status|attitude|gps|obstacle>`.

## Seguridad

- La pantalla es solo informativa: nunca controla motores ni sustituye al failsafe.
- No refrescar la pantalla dentro del lazo de control (I2C lento y bloqueante);
  usar una tarea RTOS separada.

## Verificación

- La pantalla arranca y muestra el logo/boot correctamente.
- Alternar vistas con el comando `display` del CLI.
- El refresco no introduce jitter en el bucle de control (comprobar tick a 1 kHz).
