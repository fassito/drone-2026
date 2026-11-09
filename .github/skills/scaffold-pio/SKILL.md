---
name: scaffold-pio
description: 'Crear y configurar un proyecto PlatformIO para ESP32 (DevKit V1) con framework Arduino. Use when: iniciar el proyecto del dron, crear platformio.ini, elegir placa ESP32, particiones o la estructura de carpetas del firmware.'
---

# Scaffold de proyecto PlatformIO (ESP32 + Arduino)

## Cuándo usarlo
- Crear el proyecto de firmware del dron desde cero.
- Configurar `platformio.ini` para ESP32 (DevKit V1).
- Definir la estructura de carpetas y la partición de flash.

## Procedimiento

1. Crear la estructura de carpetas:
   ```
   firmware/
   ├── platformio.ini
   ├── src/main.cpp
   ├── src/config.h
   ├── src/flight/   # mixer, pid
   ├── src/sensors/  # mpu9250
   ├── src/actuators/# esc
   ├── src/radio/    # ibus
   ├── src/nav/      # gps, obstacle, mission
   ├── src/telemetry/# wifi_cli
   ├── src/safety/   # failsafe
   ├── test/
   └── include/
   ```
2. Copiar la plantilla [platformio.ini](./assets/platformio.ini) en `firmware/platformio.ini`.
3. Verificar que la placa es `esp32dev` (**ESP32 DevKit V1**, ESP32-WROOM-32).
   ⚠️ Ojo con las confusiones de nombre:
   - "NodeMCU v3" (LoLin, CH340) es un **ESP8266** (un solo núcleo, 1 ADC, sin
     LEDC ni UART2): NO sirve para este proyecto. Esa placa dará error
     "This chip is ESP8266, not ESP32" al grabar.
   - La placa correcta es la **ESP32 DevKit V1** (ESP32-WROOM-32, 30 pines).
4. Añadir `lib_deps` solo cuando exista código que las use (evitar dependencias huérfanas).
5. Crear `src/config.h` con pines, canales y constantes por defecto.
6. Compilar con `pio run` y grabar con `pio run -t upload`.

## Verificación

- `pio run` termina sin errores.
- `pio device monitor -b 115200` muestra el arranque del firmware.
- El LED de la placa ejecuta un parpadeo de autocomprobación.

## Notas

- No escribir código de control dentro de `loop()`: usar tareas RTOS o temporizadores.
- Mantener la lógica de vuelo desacoplada del hardware para poder testearla en host (`pio test`).
