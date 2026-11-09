# Directrices del proyecto Drone-2026

## Plataforma

- **PlatformIO** con framework **Arduino** y placa **ESP32 DevKit V1**.
- Placa en PlatformIO: `esp32dev` (ESP32-WROOM-32).
- Compilar: `pio run`. Grabar: `pio run -t upload`. Monitor: `pio device monitor -b 115200`.

## Arquitectura

Subsistemas con responsabilidad única (ver README):

- `flight/` — bucle de control a 1 kHz: mezcla de motores (quad X) y PID en cascada.
- `sensors/` — IMU MPU-9250 (SPI): giroscopio/acelerómetro/magnetómetro y fusión Madgwick en el ESP32 a 1 kHz. Barómetro BMP388 (I2C) para altitud barométrica.
- `actuators/` — generación de señal OneShot125 para los ESC BLHeli 30A.
- `radio/` — decodificación iBus del receptor FS-IA6B por UART (115200 baud).
- `telemetry/` — CLI por WiFi (TCP) para diagnóstico en tierra; radio HC-12 (433/868/915 MHz) como enlace bidireccional en vuelo; pantalla OLED I2C de estado.
- `nav/` — GPS Neo 6M (NMEA), navegación por waypoints/RTH y evitación de choques por proximidad.
- `safety/` — armado, failsafe, corte de motores y monitor de batería (INA219).

El esquema completo de conexionado de todos los componentes está en
[docs/conexiones.md](docs/conexiones.md).

## Convenciones

- Código **no bloqueante**: el bucle de control usa temporizador/hrtimer; nunca `delay()` dentro del lazo de control.
- Pines, canales y constantes de hardware centralizados en `src/config.h`.
- Unidades SI en el núcleo de control (rad/s, rad, s, N·m); convertir solo en la capa de interfaz.
- Aislamiento de tareas: lectura de sensores, control y telemetría como tareas separadas del RTOS (o en núcleos distintos).

## Habilitación progresiva de periféricos

- Cada periférico tiene un flag `ENABLE_*` en `src/config.h` (MPU9250, I2C, GPS,
  HC12, IBUS, SHARP). Se activa en **1** solo cuando el componente está conectado.
- **Importante**: no inicializar un UART con su pin RX flotante (genera ruido e
  interrupciones continuas). Por eso los flags arrancan en 0 y se encienden por fase.
- Orden de montaje y test (Fase 0→7): DevKit solo → MPU-9250 → I2C → GPS → HC-12
  → iBus → Sharp → potencia/motores. Cada fase se valida con `selftest` o el
  comando CLI correspondiente.

## Núcleos (dual-core)

- **Core 0**: dedicado en exclusiva al **bucle de control de 1 kHz** (prioridad máxima).
- **Core 1**: comunicaciones (WiFi, HC-12, GPS, iBus) y sensores/UI auxiliares
  (OLED, monitor de batería, barómetro). Aquí corre el `loop()` de Arduino.
- Las tareas de comunicación se crean **ancladas al core 1** (`xTaskCreatePinnedToCore(..., 1)`)
  para no tocar el core de control.
- Las interrupciones (UART iBus/HC-12, SoftwareSerial GPS) cuestan microsegundos y no
  degradan el lazo de 1 kHz; el aislamiento por núcleo protege frente a la carga de
  tareas (parsing, JSON, dibujo OLED), no frente a interrupciones.
- WiFi encendido solo en tierra (se apaga al armar); sus tareas del driver tienen
  prioridad alta, por eso conviene mantenerlo fuera del vuelo.
- Los protocolos (iBus, OneShot125, CLI) deben tener tests unitarios desacoplados del hardware.

## Seguridad (obligatorio)

- Los motores **nunca** arrancan sin armado explícito y sin failsafe activo.
- Armado bloqueado si el throttle no está en el mínimo.
- Failsafe: pérdida de señal de radio → desarmar o aterrizaje controlado (según modo).
- Corte de motor inmediato ante ángulo fuera de límites o sobrecorriente.
- **Batería**: el monitor INA219 alimenta el failsafe de tensión baja → aterrizaje.
- **Modo híbrido WiFi**: el AP WiFi se desactiva automáticamente al armar (para no
  interferir con la FS-i6 en 2.4 GHz) y se reactiva al desarmar.
- **Enlace en vuelo**: la telemetría/comandos en vuelo van por el **HC-12**
  (433/868/915 MHz), fuera de la banda del mando.
- Pruebas de escritorio siempre **sin hélices**.

## Build y test

- `pio run` para compilar el firmware.
- `./run_host_tests.sh` para la batería de tests de host (módulos puros, sin
  hardware): compila los módulos C-compatibles y ejecuta las suites por
  componente e integración.
- En el dispositivo, ejecutar `selftest` desde la CLI (WiFi o HC-12) para
  comprobar los componentes conectados (I2C, IMU, barómetro, batería, receptor,
  GPS, proximidad).
- Cada módulo nuevo debe llevar su suite de tests en `tests/` y registrarse en
  `tests/test_main.c` y en `run_host_tests.sh`.
- Antes de entregar cambios, compilar y ejecutar los tests relevantes.
