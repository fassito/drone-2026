# Opciones de sensores de proximidad

## Sensor elegido: Sharp GP2Y0A21 (IR analógico)

- **Tipo**: infrarrojo con salida analógica.
- **Alcance**: 10–80 cm (útil para evitar choques).
- **Interfaz**: ADC (1 pin por sensor).
- **Alimentación**: 4.5–5.5 V (5 V), consumo bajo (~30–50 mA).

## Comparativa (alternativas)

| Sensor | Tipo | Alcance | Interfaz | Notas |
|---|---|---|---|---|
| **Sharp GP2Y0A21** | IR analógico | 10–80 cm | ADC | **Elegido**; barato y simple, salida no lineal |
| VL53L0X | ToF (láser) | 3–200 cm | I2C (0x29) | Preciso; requiere mux si hay varios |
| HC-SR04 | Ultrasónico | 2–400 cm | 2 GPIO (TRIG/ECHO) | Sensible a interferencias y ángulo |
| TFmini / TF-Luna | LiDAR | 30 cm–12 m | UART (115200) | Largo alcance; consume un UART |

## Configuración por defecto

- **4× Sharp GP2Y0A21** mirando a frente/derecha/atrás/izquierda.
- Conectados a canales **ADC1** del ESP32 (GPIO 36, 37, 38, 39), porque ADC2
  queda conflictivo con el WiFi activo.
- Alimentar los sensores con 5 V (desde el BEC/regulador de 5 V; no desde el 3.3 V).

```
Sharp frente   ──► GPIO 36 (ADC1)
Sharp derecha  ──► GPIO 37 (ADC1)
Sharp atrás    ──► GPIO 38 (ADC1)
Sharp izquierda──► GPIO 39 (ADC1)
GND común      ──► GND
VCC            ──► 5 V
```

## Curva de salida (no lineal)

El GP2Y0A21 entrega una tensión inversamente proporcional a la distancia
(aprox., varía entre unidades — calibrar):

| Distancia (cm) | Tensión aprox. (V) |
|---|---|
| 10 | 2.6 |
| 20 | 1.5 |
| 30 | 0.9 |
| 40 | 0.7 |
| 60 | 0.45 |
| 80 | 0.35 |

**Recomendación**: calibrar una tabla look-up (LUT) midiendo con un objeto real
a distancias conocidas (10, 20, 30, 50, 80 cm) e interpolar. Fórmula aproximada
para empezar: `distancia ≈ 4800 / (lectura_ADC − 20)` con `analogRead` de 10 bits.

## Umbrales y estados

| Umbral | Acción |
|---|---|
| `PROX_WARN_CM` (p. ej. 60 cm) | Reducir velocidad de aproximación |
| `PROX_STOP_CM` (p. ej. 30 cm) | Detener avance / mantener posición |
| Lectura inválida o fuera de rango (10–80 cm) | Ignorar y marcar sensor no fiable |

## Lógica de evitación (fusión con navegación)

1. Si frente < `PROX_STOP_CM`: detener avance; si derecha más libre que izquierda,
   desviar a la derecha (y viceversa).
2. Si cualquier dirección < umbral mínimo de seguridad: parada de emergencia.
3. Aplicar histéresis (+/− 10 cm) para evitar oscilaciones.
4. Suavizar las lecturas con media móvil/filtro paso bajo (el IR analógico es ruidoso).

