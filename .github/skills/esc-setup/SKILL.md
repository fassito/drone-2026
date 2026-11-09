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
2. **Alimentación de potencia**: la batería entra al **PDB (XT-60)**, que reparte a los
   4 ESC y al BEC de 5 V. Los ESC BLHeli 30A suelen ser OPTO (sin BEC): el ESP32,
   el receptor y los sensores se alimentan desde el BEC, no desde los ESC.
3. Conectar los 4 ESC a 4 pines GPIO con salida de alta resolución (LEDC o RMT).
4. Implementar `actuators/esc.h/cpp`:
   - `esc_arm()` / `esc_disarm()`.
   - `esc_write_us(channel, us)` con limitación de rango 125–250 µs.
   - Filtro de rampa para evitar saltos bruscos de señal.
5. **Calibración** (hélices desmontadas):
   a. Arrancar con throttle máximo (señal máxima OneShot125).
   b. Alimentar los ESC y esperar los beeps de programación.
   c. Bajar a throttle mínimo y esperar los beeps de confirmación.
   d. Almacenar los valores calibrados en `config.h`.
6. Verificar dirección de giro de cada motor y corregir en BLHeli o intercambiando dos fases.
7. Probar cada motor individualmente con el comando de test del CLI (ver skill `wifi-cli`).

## Seguridad

- **Prohibido** calibrar o probar con hélices montadas.
- Alimentar los ESC solo con una fuente limitada en corriente durante las pruebas.
- El código nunca debe emitir señal de motor sin armado explícito.

## Verificación

- Los 4 motores responden linealmente al cambio de throttle.
- Sin saltos ni arranques bruscos al pasar de 0 a mínimo.
- Direcciones de giro correctas según la mezcla de quad X.
