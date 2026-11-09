# Protocolo NMEA y matemáticas de navegación (GPS Neo 6M)

## Configuración del Neo 6M

| Dato | Valor |
|---|---|
| Baud | 9600 (por defecto) |
| Nivel lógico | 3.3 V (TTL) |
| Tensión de alimentación | 3.3–5 V según módulo |
| Antena | cerámica activa integrada |
| Salidas | `$GPGGA`, `$GPRMC`, `$GPGSA`, `$GPGSV`, `$GPVTG` |
| Frecuencia de fix | 1 Hz (configurable vía u-blox) |

## Tramas útiles

### `$GPGGA` — posición y fix
```
$GPGGA,hhmmss.ss,ddmm.mmmm,N,dddmm.mmmm,W,1,07,1.0,650.0,M,48.0,M,,*CS
```
Campos: hora, latitud (grados+minutos), N/S, longitud, E/W, **fix** (0=sin fix,
1=GPS, 2=DGPS), satélites, HDOP, altitud (m), ...

### `$GPRMC` — datos recomendados mínimos
```
$GPRMC,hhmmss.ss,A,ddmm.mmmm,N,dddmm.mmmm,W,0.5,180.0,260126,,,A*CS
```
Campos: hora, **estado A/V**, latitud, N/S, longitud, E/W, velocidad (nudos),
rumbo (grados), fecha, ...

## Conversión de coordenadas

- Formato NMEA: `ddmm.mmmm` (grados + minutos).
- A grados decimales: `grados = dd + mm.mmmm / 60`, con signo negativo si S o W.

## Distancia (haversine)

```
a = sin²(Δlat/2) + cos(lat1)·cos(lat2)·sin²(Δlon/2)
d = 2·R·atan2(√a, √(1−a))          # R = 6371000 m
```

## Rumbo al destino (bearing)

```
θ = atan2( sin(Δlon)·cos(lat2),
           cos(lat1)·sin(lat2) − sin(lat1)·cos(lat2)·cos(Δlon) )
```
Resultado en radianes, normalizado a [0, 2π). Convertir a grados para el control.

## Radio de llegada

- Destino alcanzado si `distance <= WAYPOINT_RADIUS_M` (p. ej. 2 m).
- Ajustar el radio según la precisión del GPS (2–5 m típico en exterior).

## Notas

- El Neo 6M necesita cielo despejado la primera vez (cold start) para el fix.
- **Altitud**: no usar la altitud GPS para control fino de altura (ruidosa); usar
  el **BMP388** (I2C) como referencia barométrica de altitud.
