# Protocolo iBus (FlySky FS-IA6B + FS-i6)

## Equipo

- **Emisora**: FlySky **FS-i6** (6 canales, AFHDS 2A).
- **Receptor**: FlySky **FS-IA6B** (6 canales PWM + salida iBus en el puerto
  `iBus Servo`). Compatible y enlazable con la FS-i6.

## Enlace (binding)

1. Alimentar el FS-IA6B (4.0–6.5 V, típico 5 V).
2. Mantener pulsado el botón `BIND` del receptor mientras se alimenta; el LED
   queda parpadeando.
3. En la FS-i6, mantener pulsado el botón de binding mientras se enciende.
4. El LED del receptor queda fijo cuando el enlace es correcto.

## Salida iBus

- Usar el conector **`iBus Servo`** del FS-IA6B (no el de PWM por canal).
- El FS-IA6B entrega iBus a **115200 baud, 8N1** y **nivel TTL de 5 V**.
- **Importante**: los GPIO del ESP32 son de 3.3 V y **no toleran 5 V**. Insertar
  un divisor resistivo (p. ej. 2.2 kΩ serie + 4.7 kΩ a GND ≈ 3.4 V) o un
  nivelador lógico entre la salida iBus y el GPIO 34.

## Trama

| Campo | Bytes | Valor |
|---|---|---|
| Cabecera | 2 | `0x20 0x40` |
| Datos | 30 | 14 canales × 2 bytes (little-endian) + 2 bytes de relleno |
| Checksum | 2 | `0xFFFF − suma(bytes[2..30])` (little-endian) |
| **Total** | **32** | |

## Canales

Cada canal es un entero de 16 bits en el rango nominal 1000–2000 (punto medio 1500).
El FS-IA6B expone 6 canales utilizables:

| Canal | Función |
|---|---|
| CH1 | Aileron (roll) |
| CH2 | Elevator (pitch) |
| CH3 | Throttle |
| CH4 | Rudder (yaw) |
| CH5 | Conmutador de armado |
| CH6 | Modo de vuelo |

## Decodificación

1. Buscar cabecera `0x20 0x40`.
2. Acumular hasta 32 bytes.
3. `checksum = 0xFFFF - (suma de bytes[2]..bytes[30])`; comparar con bytes[30..31] (little-endian).
4. `canal[i] = bytes[2 + i*2] | (bytes[3 + i*2] << 8)`.
5. Escalar a [-1, +1]: `value = (canal - 1500) / 500.0`, limitado a ±1.

## Failsafe de receptor

- Si no se recibe trama válida durante un umbral (p. ej. 100–250 ms), considerar pérdida de señal.
- No mantener el último valor de throttle: forzar valor seguro y activar failsafe.
