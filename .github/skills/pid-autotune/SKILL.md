---
name: pid-autotune
description: 'Implementar el PID en cascada y el autoajuste automático de parámetros para la estabilización del dron. Use when: sintonizar PID de roll/pitch/yaw, autoajuste automático (Ziegler-Nichols, relé o escalón), ajustar lazo de ángulo y de velocidad angular.'
---

# PID en cascada y autoajuste de parámetros

## Cuándo usarlo
- Implementar el controlador PID de estabilización (lazo externo de ángulo + interno de velocidad angular).
- Ajustar parámetros P/I/D por eje (roll, pitch, yaw).
- Diseñar y ejecutar el autoajuste automático.

## Procedimiento

1. **Leer** [pid-cascade.md](./references/pid-cascade.md) para la estructura y fórmulas.
   La actitud y la velocidad angular provienen de la IMU MPU-9250 (fusión
   Madgwick en el ESP32 a 1 kHz); el giroscopio entrega la velocidad angular
   directa para el lazo de rate.
2. Implementar el PID con:
   - Filtro derivativo (paso bajo sobre la derivada) para reducir ruido del giroscopio.
   - Anti-windup por limitación de la integral.
   - Salida limitada al rango del mixer.
3. Empezar con el lazo de **velocidad angular** (rate) y después cerrar el lazo de **ángulo**.
4. Autoajuste por eje (solo en modo rate y con el dron fijo a un banco de pruebas o en vuelo estabilizado):
   - Método de **relé/oscilación** o **escalón** para estimar ganancia crítica y periodo.
   - Aplicar reglas de **Ziegler-Nichols** como punto de partida.
   - Refinar con barridos incrementales y métrica de error (IAE/ISE) sobre los datos registrados.
5. Registrar telemetría con `log start` (ver skill `wifi-cli`) y analizar la respuesta.
6. Guardar parámetros en NVS con `pid save`.

## Seguridad

- Autoajuste solo en modo rate, con límites de amplitud y de duración.
- Umbral de divergencia: si la actitud supera un límite, abortar y restaurar los parámetros previos.
- Realizar las primeras pasadas en banco de pruebas o con el dron atado, **sin hélices** cuando sea posible.

## Verificación

- Respuesta al escalón sin sobreoscilación excesiva y sin oscilación sostenida.
- Los parámetros convergen en varias pasadas y se guardan correctamente.
- El límite de divergencia aborta el autoajuste y restaura el PID anterior.
