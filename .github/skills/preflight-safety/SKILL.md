---
name: preflight-safety
description: 'Revisión de seguridad prevuelo y de failsafe del dron. Use when: revisar failsafe, armado seguro, checklist prevuelo, corte de motores, pruebas de desconexión de radio o WiFi.'
---

# Checklist de seguridad prevuelo

## Cuándo usarlo
- Antes de cualquier prueba con motores o vuelo.
- Revisar armado, failsafe y corte de motores.
- Validar comportamientos de seguridad tras cambios de firmware.

## Checklist prevuelo

1. **Hélices desmontadas** para toda prueba en banco.
2. Batería cargada y fijada; comprobar tensión (3–6S).
3. Receptor FS-IA6B enlazado (LED fijo) y canales con valores correctos en el CLI (`status`).
4. IMU MPU-9250 conectada por SPI y calibrada (giroscopio sin drift en reposo); valores estables en `attitude`.
5. GPS con fix válido (≥ 4 satélites) en exterior (`gps`), si se va a volar en modo autónomo.
6. Sensores de proximidad verificados: `obstacle` muestra distancias y un objeto cercano activa la frenada.
7. Failsafe de radio probado: apagar la emisora → el dron desarma o aterriza.
8. Failsafe de WiFi probado: desconectar el cliente → sin comandos de motor activos.
9. Return-to-home (RTH) probado en espacio abierto y sin obstáculos.
10. Armado bloqueado con throttle ≠ mínimo (verificar que no arma).
11. `motortest` verificado motor a motor (sin hélices).

## Comportamientos de failsafe obligatorios

| Evento | Acción |
|---|---|
| Pérdida de señal iBus | Desarmar (o aterrizaje controlado en vuelo) |
| Desconexión del CLI WiFi | Mantener modo seguro; bloquear comandos de motor |
| Ángulo fuera de límites | Corte de motores |
| Tensión baja | Aviso y aterrizaje |
| Armado con throttle ≠ mínimo | Rechazar armado |
| Sin fix GPS en misión autónoma | Abortar misión; mantener posición o RTH si hay fix |
| Obstáculo en rango crítico | Parada de emergencia / desvío |

## Regla de oro

> Si no se puede verificar un elemento del checklist, **no** se pasa a la siguiente fase de pruebas.
