# Temporización OneShot125

Protocolo de señal ESC analógica de alta frecuencia usado por BLHeli.

## Rango de pulso

| Estado | Duración de pulso |
|---|---|
| Mínimo (0 % throttle) | 125 µs |
| Máximo (100 % throttle) | 250 µs |
| Punto medio | 187.5 µs |

## Características

- Frecuencia máxima de refresco: hasta ~8 kHz (periodo mínimo ~125 µs).
- El ESP32 genera la señal con el periférico **LEDC** (PWM) o **RMT** para precisión de µs.
- Con LEDC: usar resolución de bits y frecuencia que permitan pasos de 1 µs.
  - Ejemplo: LEDC 80 MHz, resolución de 15 bits → paso de 0.0125 µs.

## Reglas de implementación

1. Inicializar siempre con salida **desarmada** (sin pulsos / nivel bajo).
2. Limitar la escritura al rango `[125, 250]` µs.
3. Aplicar rampa de aceleración para no dañar ESC/motores.
4. Nunca emitir señal antes de que el failsafe esté activo.

## Calibración típica (hélices desmontadas)

1. Enviar pulso máximo (250 µs).
2. Alimentar los ESC → beeps de entrada a programación.
3. Enviar pulso mínimo (125 µs) → beeps de confirmación.
4. Guardar el rango calibrado.
