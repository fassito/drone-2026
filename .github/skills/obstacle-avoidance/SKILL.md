---
name: obstacle-avoidance
description: 'Integrar sensores de proximidad y lógica de evitación de choques para el dron ESP32. Use when: conectar sensores de distancia (Sharp GP2Y0A21, VL53L0X ToF, HC-SR04 ultrasónico, TFmini LiDAR), detectar obstáculos, frenar/esquivar o parada de emergencia ante colisión.'
---

# Sensores de proximidad y evitación de choques

## Cuándo usarlo
- Elegir y cablear sensores de proximidad para el dron.
- Leer distancias de obstáculos en una o varias direcciones.
- Implementar la lógica de evitación: frenar, desviar o parada de emergencia.

## Procedimiento

1. **Leer** [sensor-options.md](./references/sensor-options.md) para el sensor elegido y su conexión.
2. **Configuración**: 4× Sharp GP2Y0A21 (IR analógico, 10–80 cm) mirando a
   frente/derecha/atrás/izquierda, conectados a los canales **ADC1** (GPIO 36–39),
   que no entran en conflicto con el WiFi activo.
3. Implementar `nav/obstacle.*`:
   - `obstacle_init()`: inicializa el ADC y configura los pines de los 4 sensores.
   - `obstacle_read()`: convierte tensión → distancia con una **tabla LUT calibrada**
     (la curva del Sharp es no lineal y varía entre unidades), con media móvil.
4. Definir umbrales:
   - `PROX_WARN_CM`: reducir velocidad.
   - `PROX_STOP_CM`: detener y mantener posición.
   - Histéresis para evitar oscilaciones entre estados.
   - Lecturas fuera del rango útil (10–80 cm) → sensor no fiable.
5. Fusión con la navegación (ver skill `gps-nav`):
   - Obstáculo delante → detener o desviar (elegir lado con más espacio libre).
   - Obstáculo muy cercano → parada de emergencia (sin tocar la lógica de failsafe).

## Seguridad

- La parada por obstáculo **no** debe desactivar el failsafe; son capas independientes.
- Los sensores deben verificarse antes de cada vuelo autónomo (ver skill `preflight-safety`).
- Nunca confiar en un único sensor para seguridad crítica; validar lecturas (fuera de rango → ignorar).

## Verificación

- `obstacle` en el CLI muestra las distancias de las 4 direcciones.
- Acercar un objeto → `obstacle_near` se activa y el dron frena/se detiene.
- Sin lecturas válidas → el modo autónomo queda bloqueado.
