---
name: esc-setup
description: 'Configurar y calibrar ESC BLHeli 30A con protocolo OneShot125 para el dron ESP32. Use when: generar señal OneShot125, calibrar rango de throttle de los ESC, configurar BLHeli, pruebas de motores sin hélices.'
---

# Configuración de ESC BLHeli 30A + OneShot125

## Cuándo usarlo
- Generar la señal OneShot125 desde el ESP32 para los 4 ESC.
- Calibrar el rango de throttle (mínimo/máximo) de los ESC.
- Configurar parámetros BLHeli (timing, demag, dirección de giro).
- Hacer pruebas de giro de motores **siempre sin hélices**.

## Procedimiento

1. **Leer** [oneshot125-timing.md](./references/oneshot125-timing.md) para los rangos de pulso.
   Para programar/clonar la configuración de los ESC, leer
   [blheli-programming.md](./references/blheli-programming.md).
2. **Alimentación de potencia**: la batería entra al **PDB (XT-60)**, que reparte a los
   4 ESC y al BEC de 5 V. Los ESC BLHeli 30A suelen ser OPTO (sin BEC): el ESP32,
   el receptor y los sensores se alimentan desde el BEC, no desde los ESC.
3. Conectar los 4 ESC a 4 pines GPIO con salida de alta resolución (LEDC o RMT).
4. Implementar `actuators/esc.h/cpp`:
   - `esc_arm()` / `esc_disarm()`.
   - `esc_write_us(channel, us)` con limitación de rango 125–250 µs.
   - Filtro de rampa para evitar saltos bruscos de señal.
5. **Configuración de los ESC** (hélices desmontadas):
   - Con **OneShot125 no hay calibración de recorrido**: el rango es fijo
     (125–250 µs). Solo hay que poner el protocolo en OneShot125 en BLHeli.
   - Clonar la misma configuración a los 4 ESC con BLHeliSuite32 y un `.ini`
     guardado (ver `references/blheli-programming.md`).
   - La calibración máximo→mínimo solo se hace si el ESC queda en PWM estándar.
6. Verificar dirección de giro de cada motor y corregir en BLHeli o intercambiando dos fases.
7. Probar cada motor individualmente con `motortest <1-4> <0-100> [ms]`
   (y `motortest stop`) desde el CLI. Solo con el dron desarmado.

## Seguridad

- **Prohibido** calibrar o probar con hélices montadas.
- Alimentar los ESC solo con una fuente limitada en corriente durante las pruebas.
- El código nunca debe emitir señal de motor sin armado explícito.

## Verificación

- Los 4 motores responden linealmente al cambio de throttle.
- Sin saltos ni arranques bruscos al pasar de 0 a mínimo.
- Direcciones de giro correctas según la mezcla de quad X.
