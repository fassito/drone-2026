# Catálogo de comandos del CLI

Comandos previstos para la interfaz WiFi del dron.

| Comando | Argumentos | Descripción |
|---|---|---|
| `help` | — | Lista comandos disponibles |
| `wifi` | `on` / `off` | Activa/desactiva el AP WiFi manualmente (diagnóstico) |
| `status` | — | Estado: armado, modo, tensión, fallos |
| `attitude` | — | Última actitud estimada (roll/pitch/yaw) |
| `motors` | — | Señal actual de los 4 motores |
| `motortest <n> <pwm>` | motor y valor | Gira un motor (requiere confirmación, sin hélices) |
| `arm` | `confirm` | Arma el dron (throttle en mínimo + failsafe OK) |
| `disarm` | — | Desarma y corta motores |
| `pid` | — | Muestra parámetros PID actuales |
| `pid set <eje> <P> <I> <D>` | — | Ajusta un parámetro PID |
| `pid save` | — | Guarda PID en NVS |
| `calibrate` | `gyro` / `mag` | Calibra giroscopio o magnetómetro |
| `autotune` | `start` / `stop` | Inicia/detiene autoajuste PID |
| `gps` | — | Fix, satélites, lat/lon/alt y rumbo |
| `mission` | `set <lat> <lon>` / `stop` | Fija destino autónomo o aborta misión |
| `rth` | `confirm` | Return-to-home a la posición de armado |
| `obstacle` | — | Distancias de los sensores de proximidad |
| `geofence` | — | Estado de los límites de distancia/altitud |
| `blackbox` | — | Vuelca el registro circular de vuelo (CSV) |
| `json` | — | Telemetría en una línea JSON |
| `display` | `status` / `attitude` / `gps` / `obstacle` | Alterna la vista de la pantalla OLED |
| `log` | `start` / `stop` | Activa registro de telemetría |
| `reboot` | — | Reinicia el ESP32 |

## Reglas

- Comandos sensibles (`arm`, `motortest`, `calibrate esc`, `autotune start`,
  `mission set`, `rth`, `wifi off`) exigen confirmación o doble verificación.
- La salida usa texto plano legible; para telemetría estructurada usar JSON opcional (`json`).
