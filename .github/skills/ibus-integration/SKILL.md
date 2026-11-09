---
name: ibus-integration
description: 'Integrar el receptor FlySky FS-IA6B y decodificar el protocolo iBus por UART en el ESP32 (emisora FS-i6). Use when: leer canales de radio, parsear tramas iBus, mapear canales a roll/pitch/yaw/throttle, detectar pérdida de señal (failsafe de receptor).'
---

# Integración del receptor FS-IA6B (iBus)

## Cuándo usarlo
- Conectar el receptor FS-IA6B (emisora FlySky FS-i6) al ESP32 y leer sus 6 canales.
- Enlazar la FS-i6 con el FS-IA6B y configurar la salida iBus.
- Decodificar tramas iBus por UART a 115200 baud.
- Mapear canales a comandos de vuelo y detectar pérdida de señal.

## Procedimiento

1. **Leer** [ibus-protocol.md](./references/ibus-protocol.md) para el formato de trama y el enlace FS-i6 ↔ FS-IA6B.
2. Cableado típico:
   - FS-IA6B **iBus Servo** → GPIO RX del ESP32 con **divisor/nivelador 5 V → 3.3 V**
     (los GPIO del ESP32 no toleran 5 V; la placa de desarrollo NO resuelve esto).
   - Alimentar el receptor con 5 V regulados.
3. Inicializar `Serial2` a 115200 baud y leer byte a byte con máquina de estados.
4. Validar cabecera `0x20 0x40`, longitud 32 y checksum (0xFFFF − suma).
5. Escalar cada canal (1000–2000 µs, 16 bits) a [-1, +1] o a unidades del controlador.
6. Mapear canales del FS-IA6B:
   - CH1–CH4: Aileron (roll), Elevator (pitch), Throttle, Rudder (yaw).
   - CH5: conmutador de armado.
   - CH6: modo de vuelo (ángulo / rate / autotune).
7. Implementar **failsafe de receptor**: si no llega trama válida durante N ms, marcar pérdida de señal y activar el failsafe de `safety/`.

## Seguridad

- El armado exige canal de throttle en mínimo y conmutador CH5 en posición de armado.
- Ante pérdida de señal: desarmar o aterrizaje controlado según el modo configurado.

## Verificación

- Capturar tramas y comprobar checksum y valores estables en reposo.
- Prueba unitaria del parser con una trama iBus de ejemplo.
- Test de desconexión: retirar la señal y verificar que se activa el failsafe.
