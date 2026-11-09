---
name: gps-nav
description: 'Integrar el GPS Neo 6M-0-001 y navegación autónoma por waypoints en el dron ESP32. Use when: parsear NMEA, leer posición GPS, calcular rumbo y distancia (haversine), waypoints, return-to-home o llegar de forma autónoma a un destino.'
---

# GPS Neo 6M + navegación autónoma

## Cuándo usarlo
- Conectar el GPS Neo 6M-0-001 al ESP32 y leer posición.
- Parsear tramas NMEA (GPGGA, GPRMC) por UART a 9600 baud.
- Calcular rumbo (bearing) y distancia al destino.
- Implementar waypoints, return-to-home (RTH) y llegada autónoma.

## Procedimiento

1. **Leer** [nmea-protocol.md](./references/nmea-protocol.md) para el formato NMEA y las fórmulas de navegación.
2. **Cableado** (el GPS va por SoftwareSerial; los UART de hardware están en
   iBus y HC-12):
   - Usar SoftwareSerial (`EspSoftwareSerial`) a 9600 baud para el Neo 6M.
   - GPS TX → RX del ESP32 (pin de solo entrada, p. ej. GPIO 35).
   - GPS VCC → 3.3 V (o 5 V según módulo), GND común.
   - Si en el futuro se libera un UART de hardware, preferirlo por fiabilidad.
3. Inicializar el puerto y parsear `$GPGGA` (fix, lat/lon, altitud, satélites) y `$GPRMC` (velocidad, rumbo).
4. Esperar fix válido (`fix >= 1`) antes de usar la posición; marcar `gps_state_t.fix`.
5. En el primer armado con fix, guardar la posición como **home** (para RTH).
6. Navegación a destino:
   - `bearing = atan2(sin(Δlon)·cos(lat2), cos(lat1)·sin(lat2) − sin(lat1)·cos(lat2)·cos(Δlon))`.
   - `distance` por haversine.
   - Generar setpoints de rumbo/velocidad/altitud para el controlador de vuelo (no mover motores aquí).
7. Considerar el destino alcanzado cuando la distancia < radio de llegada (p. ej. 2 m).

## Seguridad

- Sin fix GPS → no se puede navegar en modo autónomo (mantener posición o RTH imposible → failsafe).
- Los waypoints deben estar dentro de límites geográficos razonables (geofence).
- RTH ante pérdida de señal de radio, si el enlace y el fix lo permiten.

## Verificación

- `gps` en el CLI muestra fix, satélites, lat/lon/alt correctos en exterior.
- El rumbo calculado apunta al destino (probar con coordenadas conocidas).
- RTH devuelve el dron a la posición de armado en pruebas controladas.
