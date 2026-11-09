---
name: wifi-cli
description: 'Implementar la interfaz de línea de comandos por WiFi (TCP) para diagnóstico, pruebas y telemetría del dron ESP32. Use when: crear comandos CLI, servidor TCP/WebSocket, telemetría en tiempo real, probar motores o PID desde el ordenador.'
---

# CLI por WiFi para diagnóstico y telemetría

## Cuándo usarlo
- Crear el servidor TCP (o WebSocket) de comandos en el ESP32.
- Definir comandos de diagnóstico, pruebas de motores y ajuste de PID.
- Enviar telemetría en tiempo real a un cliente conectado.

## Modo híbrido (WiFi solo en tierra)

El WiFi es un canal de **diagnóstico en tierra**, no un enlace de vuelo:

- Al **armar**, el WiFi se **desactiva automáticamente** para no interferir con la
  emisora FS-i6 (ambas en 2.4 GHz) y para liberar el ADC2.
- Al **desarmar**, se reactiva (o se reactiva manualmente con `wifi on`).
- La CLI en vuelo no está disponible; el control depende solo del enlace de radio
  FS-i6 y de los modos autónomos (RTH, misión).

## Procedimiento

1. **Leer** [command-list.md](./references/command-list.md) para el catálogo de comandos.
2. Configurar el ESP32 como punto de acceso (AP) con contraseña, y opcionalmente como STA para unirse a una red.
3. Inicializar un servidor TCP en un puerto fijo (p. ej. 23 o 8080) en una tarea RTOS dedicada.
4. Implementar un parser de comandos simple: `<comando> [arg1] [arg2] ...`, insensible a mayúsculas.
5. Añadir un comando `help` que liste todos los comandos disponibles.
6. Publicar telemetría periódica (actitud, throttle, PID, estado de armado) solo a clientes suscritos.
7. Proteger comandos peligrosos (`arm`, `motortest`, `calibrate`) con confirmación y con el estado de failsafe.
8. Conectar el evento de armado/desarmado al estado del WiFi: `WiFi.softAPdisconnect(true)` /
   apagar el AP al armar y reactivarlo al desarmar.

## Seguridad

- El AP WiFi debe tener contraseña; nunca red abierta.
- `arm` y `motortest` requieren confirmación explícita (p. ej. `arm confirm`).
- El failsafe de WiFi se activa si el cliente de telemetría se desconecta durante una prueba.

## Verificación

- Conectar por `nc` o `telnet` y ejecutar `help`, `status`, `pid` y `motortest`.
- Comprobar que la telemetría llega con timestamps correctos.
- Verificar que los comandos peligrosos se bloquean sin confirmación.
