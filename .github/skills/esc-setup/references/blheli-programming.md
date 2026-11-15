# Programar y clonar la configuración de los ESC BLHeli

Objetivo: que los **4 ESC queden con exactamente la misma configuración** y
que esa configuración sea la adecuada para este dron (OneShot125, 2205 2300 KV,
4S, 5").

## 1. Identificar el firmware del ESC (paso imprescindible)

No todos los "BLHeli 30 A" son iguales y **la herramienta cambia**:

| Firmware | MCU típica | Herramienta | ¿OneShot125? |
|---|---|---|---|
| **BLHeli_S** | EFM8BB1/BB2 (Silabs) | BLHeliSuite32 / BLHeli Configurator | **Sí** (nativo) |
| BLHeli (original) | Atmel ATmega / SiLabs | BLHeliSuite (4-way) | Solo en algunas revisiones SiLabs |
| BLHeli_32 | ARM | **BLHeliSuite32** (exclusivo) | Sí (DShot, OneShot…) |

Se reconoce al conectar: BLHeliSuite lee la MCU y muestra la familia
(`BLHeli_S`, `BLHeli_32`…). Si el ESC **no** soporta OneShot125, hay que
cambiarlo o usar PWM estándar (1000–2000 µs), y entonces el firmware necesita
otro modo de señal.

## 1b. Modelo concreto de este proyecto: DYS XM30A (XM mini 30 A V2)

Es un ESC de la **familia BLHeli_S** (MCU SiLabs **EFM8BB21**), con soporte
confirmado de **OneShot125** y **Damped Light**. Por tanto:

- **NO necesita BLHeliSuite32** (esa herramienta es solo para BLHeli_32/ARM).
- La herramienta adecuada en Linux es **esc-configurator.com** (BLHeli_S).
- Es **OPTO** (sin BEC): la electrónica se alimenta del BEC externo de 5 V.
- Lleva condensador electrolítico propio en los cables de alimentación.

> Comprobar la serigrafía del ESC para el rango de batería. La serie XM admite
> 4S sin problema, que es lo que usa este dron.

## 2. Hardware para programar

Los ESC van sueltos (no en una placa 4-en-1), así que hace falta un enlace:

- **BLHeliLink / USB linker** (CP2102 + resistencia) → conector de 3 hilos del ESC
  (GND, señal, GND), **sin batería** si el ESC es OPTO con BEC de programación.
- Alternativa: **Arduino UNO como 4-way interface** (BLHeliSuite tiene el
  firmware "Arduino 4way-if").

Conexión del linker al ESC: `GND – Signal – GND` (el ESC se alimenta del enlace
para leer/escribir la flash).

## 3. Clonar la configuración a los 4 ESC

1. Conecta **un** ESC al linker, abre BLHeliSuite32 → `Read Setup`.
2. Comprueba que lee la MCU y la versión de firmware.
3. Ajusta los parámetros (tabla de abajo) → `Write Setup`.
4. **Guarda la configuración**: `File → Save Settings` → `esc-drone-2026.ini`.
5. Conecta los otros 3 ESC **uno a uno** y, en cada uno:
   - `Read Setup` (para verificar que responde),
   - `Load Settings` → `esc-drone-2026.ini`,
   - `Write Setup`.
6. Vuelve a cada ESC y haz `Read Setup` para **verificar** que los 4 devuelven
   los mismos valores.

> Atajo: muchos ESC BLHeli_S se pueden programar por el **passthrough del
> controlador de vuelo**. Aquí NO hay controladora de vuelo (el ESP32 es el
> controlador), así que hace falta un adaptador **4-way interface**:
> un Arduino Nano/Uno con el firmware `4way-if`, o un dongle BLHeliLink.
> Con cualquiera de los dos funciona esc-configurator por Web Serial.

## 4. Configuración recomendada (dron de 5", 2205 2300 KV, 4S)

| Parámetro | Valor | Motivo |
|---|---|---|
| **PWM/Signal protocol** | **OneShot125** | Es lo que genera el firmware del ESP32 (125–250 µs) |
| **Motor direction** | Normal / Reversed según mezcla | Quad X: FR y RL en un sentido, FL y RR en el otro |
| **Motor timing** | **Medium** | Correcto para 2300 KV en 5" |
| **Demag compensation** | **Off** (o Low) | Evita cortes falsos; con Off y buen timing hay menos riesgo |
| **PWM frequency / Damped** | **Damped Light** (High) | Frenado activo: respuesta más precisa del lazo PID |
| **Brake on stop** | **Off** | Frenar en seco puede dañar el ESC |
| **Low voltage limit** | **Off** | La gestión de batería la hace el firmware (INA219 + failsafe) |
| **Temperature protection** | On, 140 °C | Red de seguridad |
| **Beep strength / Beacon** | Bajo / Off | Evita pitidos y consumo innecesario |
| **Programming by TX** | **Off** | Impide cambiar parámetros por error con el mando |
| **Bidirectional / Telemetry** | Off | No usamos DShot ni telemetría del ESC |

## 5. Calibración de throttle: NO hace falta con OneShot125

Con OneShot125 el rango es **fijo por protocolo** (125 µs = 0 %,
250 µs = 100 %). No hay calibración de recorrido como en PWM.

Esa calibración (máximo → alimentar → mínimo) solo se hace si el ESC está en
**PWM estándar**. Si algún día se usa PWM, entonces sí: `PWM frequency` alto y
calibrar los 4 a la vez desde el firmware.

## 6. Verificación en banco (SIN HÉLICES)

1. Flashea el firmware con los ESC conectados a GPIO 25/26/14/15.
2. Al arrancar, el firmware emite **125 µs continuos** → cada ESC debe dar el
   pitido de "throttle mínimo detectado / listo".
3. Por el CLI (WiFi o HC-12):
   ```
   motortest 1 10        # motor 1 al 10 % durante 1.5 s
   motortest 2 10
   motortest 3 10
   motortest 4 10
   motortest stop
   ```
4. Comprueba:
   - Todos arrancan a la vez con el mismo mando (configuración idéntica).
   - Giran en la dirección que espera la mezcla quad X.
   - Arrancan suave y sin tirones (rampa de 125 ms).

Si un motor gira al revés: cámbialo en BLHeli (`Motor direction`) **o**
intercambia dos de los tres cables de fase. Elige un criterio y aplícalo igual
a los cuatro para no liarte.

## 7. Seguridad

- **Siempre sin hélices** al programar, calibrar y probar.
- Sujeta el dron o los motores en banco.
- Fuente de alimentación **limitada en corriente** durante las pruebas.
- El firmware nunca genera señal por encima del mínimo sin armado explícito.
