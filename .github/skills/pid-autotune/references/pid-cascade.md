# PID en cascada para cuadricóptero

## Estructura

```
setpoint_ángulo ──► PID ángulo ──► setpoint_rate ──► PID rate ──► mixer ──► motores
        ▲                                                              │
        └────────────────── estimación de actitud ◄────────────────────┘
```

- **Lazo externo (ángulo)**: corrige el error de actitud (roll/pitch/yaw).
- **Lazo interno (rate)**: corrige la velocidad angular (más rápido y estable).

> La actitud (yaw/pitch/roll) y la velocidad angular las entrega la IMU
> **MPU-9250** (SPI): la fusión Madgwick se ejecuta en el ESP32 a 1 kHz y el
> giroscopio da la velocidad angular directa para el lazo de rate.

## PID estándar (forma paralela)

```
u(t) = Kp·e(t) + Ki·∫e(τ)dτ + Kd·(de(t)/dt)
```

Implementación discreta típica (dT = 1 ms a 1 kHz):

```
error = setpoint - medición
integral += error * dT                       # con anti-windup
derivada = (error - error_anterior) / dT     # con filtro paso bajo
salida = Kp*error + Ki*integral + Kd*derivada
```

## Pautas de ajuste

| Parámetro | Efecto | Señal de exceso |
|---|---|---|
| Kp | Respuesta proporcional | Oscilación sostenida |
| Ki | Elimina error estacionario | Sobreimpulso lento, windup |
| Kd | Amortigua, reduce sobreoscilación | Amplifica ruido |

## Autoajuste (Ziegler-Nichols, lazo cerrado)

1. Con solo P (I=0, D=0), aumentar Kp hasta oscilación sostenida → ganancia crítica `Ku` y periodo `Tu`.
2. Punto de partida:
   - PID clásico: `Kp = 0.6·Ku`, `Ti = 0.5·Tu`, `Td = 0.125·Tu` → `Ki = Kp/Ti`, `Kd = Kp·Td`.
3. Refinar con métrica de error (IAE/ISE) sobre telemetría registrada.
4. Ajustar el lazo interno (rate) primero; después el externo (ángulo).

## Seguridad en autoajuste

- Límite de amplitud en el setpoint de prueba.
- Detección de divergencia por actitud/velocidad angular fuera de rango.
- Guardar y restaurar los parámetros previos ante fallo.
