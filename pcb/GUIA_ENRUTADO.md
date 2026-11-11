# Guía de enrutado — placa Drone-2026

Placa de **90 × 90 mm**, 2 capas (`F.Cu` / `B.Cu`). Los componentes ya están colocados y las redes asignadas: solo faltan las pistas.

Coordenadas en mm con el origen en la esquina superior izquierda (Y crece hacia abajo, como en KiCad).


## Antes de empezar

1. **GND no aparece aquí**: se resuelve con un plano de masa. En el editor de PCB: `Añadir zona rellena` → capa `B.Cu` → red `GND` → dibuja el rectángulo del contorno → `B` para rellenar. (También puedes usar `pcb/add_gnd_zone.py`.)
2. Traza en el orden de esta guía: primero potencia (ancha), luego buses y por último señales.
3. Atajo en KiCad: pulsa `X` para enrutar, `V` para poner una vía, `B` para rellenar zonas.

## Redes a conectar

### 1. `VBAT` — pista de 1.5 mm  (110 mm en total)

*Potencia: batería → ESC. Lo más corto y ancho posible.*

ESC3.2 (53.4, 49.0)  →  ESC2.2 (46.3, 49.0)  →  ESC1.2 (39.2, 49.0)  →  ESC4.2 (60.5, 49.0)  →  XT1.1 (51.3, 67.7)  →  U3.5 (52.7, 14.0)

### 2. `5V` — pista de 0.8 mm  (248 mm en total)

*Alimentación de la electrónica desde el BEC.*

LS1.1 (14.1, 8.3)  →  J1.29 (6.3, 39.4)  →  U8.1 (10.9, 46.4)  →  U9.1 (18.0, 46.4)  →  U10.1 (25.1, 46.4)  →  U11.1 (32.2, 46.4)  →  D1.1 (37.4, 67.2)  →  C6.1 (30.7, 77.1)  →  C5.1 (21.6, 77.1)  →  C4.1 (12.6, 77.1)  →  BEC1.3 (3.8, 51.5)  →  J2.1 (67.6, 46.4)  →  U7.1 (59.7, 3.8)

### 3. `3V3` — pista de 0.8 mm  (296 mm en total)

*Alimentación de sensores desde el regulador del DevKit.*

U2.1 (45.6, 3.8)  →  U5.1 (38.5, 3.8)  →  U3.1 (52.7, 3.8)  →  U2.7 (45.6, 19.0)  →  U4.1 (76.8, 3.8)  →  U6.1 (83.9, 3.8)  →  R4.2 (76.2, 60.9)  →  C2.1 (77.2, 67.4)  →  C1.1 (68.2, 67.4)  →  R5.2 (10.7, 67.4)  →  C3.1 (3.5, 77.1)  →  J1.30 (3.8, 39.4)  →  J1.2 (3.8, 3.8)

### 4. `I2C_SDA` — pista de 0.3 mm  (155 mm en total)

*Rutear junto a I2C_SCL, en paralelo.*

U2.6 (45.6, 16.5)  →  U3.4 (52.7, 11.4)  →  U4.4 (76.8, 11.4)  →  R4.1 (68.6, 60.9)  →  J1.22 (3.8, 29.2)

### 5. `I2C_SCL` — pista de 0.3 mm  (144 mm en total)

*Rutear junto a I2C_SDA, en paralelo.*

U2.4 (45.6, 11.4)  →  U3.3 (52.7, 8.9)  →  U4.3 (76.8, 8.9)  →  J1.24 (3.8, 31.7)  →  R5.1 (3.1, 67.4)

### 6. `SPI_SCK` — pista de 0.3 mm  (38 mm en total)

*Bus SPI de la IMU: mantener corto.*

J1.18 (3.8, 24.1)  →  U5.3 (38.5, 8.9)

### 7. `SPI_MOSI` — pista de 0.3 mm  (42 mm en total)

*Bus SPI de la IMU.*

J1.26 (3.8, 34.3)  →  U5.4 (38.5, 11.4)

### 8. `SPI_MISO` — pista de 0.3 mm  (36 mm en total)

*Bus SPI de la IMU.*

J1.20 (3.8, 26.7)  →  U5.7 (38.5, 19.0)

### 9. `SPI_CS_IMU` — pista de 0.3 mm  (35 mm en total)

*Bus SPI de la IMU.*

J1.16 (3.8, 21.6)  →  U5.9 (38.5, 24.1)

### 10. `IBUS_RAW` — pista de 0.3 mm  (8 mm en total)

*Señal del receptor al divisor.*

J2.3 (67.6, 51.5)  →  R1.1 (74.0, 46.2)

### 11. `IBUS_RX` — pista de 0.3 mm  (129 mm en total)

*Salida del divisor al GPIO34.*

J1.7 (6.3, 11.4)  →  R2.1 (3.1, 60.9)  →  R1.2 (81.6, 46.2)

### 12. `MOTOR1` — pista de 0.3 mm  (44 mm en total)

*Señal OneShot125 al ESC.*

J1.15 (6.3, 21.6)  →  ESC1.3 (39.2, 51.5)

### 13. `MOTOR2` — pista de 0.3 mm  (48 mm en total)

*Señal OneShot125 al ESC.*

J1.17 (6.3, 24.1)  →  ESC2.3 (46.3, 51.5)

### 14. `MOTOR3` — pista de 0.3 mm  (52 mm en total)

*Señal OneShot125 al ESC.*

ESC3.3 (53.4, 51.5)  →  J1.21 (6.3, 29.2)

### 15. `MOTOR4` — pista de 0.3 mm  (60 mm en total)

*Señal OneShot125 al ESC.*

J1.19 (6.3, 26.7)  →  ESC4.3 (60.5, 51.5)

### 16. `GPS_TX` — pista de 0.3 mm  (80 mm en total)

*UART del GPS.*

U6.3 (83.9, 8.9)  →  J1.10 (3.8, 14.0)

### 17. `GPS_RX` — pista de 0.3 mm  (78 mm en total)

*UART del GPS.*

U6.4 (83.9, 11.4)  →  J1.9 (6.3, 14.0)

### 18. `HC12_TX` — pista de 0.3 mm  (57 mm en total)

*UART2 del HC-12.*

J1.14 (3.8, 19.0)  →  U7.3 (59.7, 8.9)

### 19. `HC12_RX` — pista de 0.3 mm  (56 mm en total)

*UART2 del HC-12.*

J1.12 (3.8, 16.5)  →  U7.4 (59.7, 11.4)

### 20. `HC12_SET` — pista de 0.3 mm  (57 mm en total)

*Modo AT del HC-12.*

J1.25 (6.3, 34.3)  →  U7.5 (59.7, 14.0)

### 21. `SHARP_FR` — pista de 0.3 mm  (45 mm en total)

*Analógica del sensor frontal.*

U8.3 (10.9, 51.5)  →  J1.3 (6.3, 6.3)

### 22. `SHARP_R` — pista de 0.3 mm  (37 mm en total)

*Analógica del sensor derecho.*

J1.11 (6.3, 16.5)  →  U9.3 (18.0, 51.5)

### 23. `SHARP_B` — pista de 0.3 mm  (38 mm en total)

*Analógica del sensor trasero.*

J1.13 (6.3, 19.0)  →  U10.3 (25.1, 51.5)

### 24. `SHARP_L` — pista de 0.3 mm  (50 mm en total)

*Analógica del sensor izquierdo.*

J1.5 (6.3, 8.9)  →  U11.3 (32.2, 51.5)

### 25. `LED_PIN` — pista de 0.3 mm  (66 mm en total)

*LED de estado.*

J1.8 (3.8, 11.4)  →  D2.2 (69.5, 5.2)

### 26. `BUZZER_PIN` — pista de 0.3 mm  (62 mm en total)

*GPIO del buzzer a la base del transistor.*

J1.6 (3.8, 8.9)  →  R3.1 (38.3, 60.9)

### 27. `BUZZER_GATE` — pista de 0.3 mm  (30 mm en total)

*Base del transistor a la resistencia.*

Q1.1 (16.8, 68.7)  →  R3.2 (46.0, 60.9)

### 28. `BUZZER_LOW` — pista de 0.3 mm  (86 mm en total)

*Colector del transistor al buzzer y al diodo.*

LS1.2 (21.7, 8.3)  →  Q1.3 (19.3, 68.7)  →  D1.2 (45.0, 67.2)

### 29. `BEC_IN` — pista de 1.0 mm  (57 mm en total)

*Entrada del BEC, a través del shunt del INA219.*

BEC1.1 (3.8, 46.4)  →  U3.6 (52.7, 16.5)


## Consejos por bloque

- **VBAT**: de XT1 a los 4 ESC y a INA219. Es la de más corriente: usa 1.5 mm o más y evita rodeos.
- **BEC_IN**: pasa por el shunt del INA219 (Vin+ / Vin−). Respétalo o la medición de corriente no funciona.
- **I2C_SDA / SCL**: trázalas en paralelo y cortas; compartidas por OLED, BMP388 e INA219.
- **SPI**: SCK/MOSI/MISO/CS de la IMU al DevKit. Mantenlas juntas y lejos de VBAT para evitar ruido.
- **MOTOR1-4**: solo señal, 0.3 mm. Van del DevKit a cada ESC.
- **Divisor iBus**: R1 (2k2) en serie desde IBUS_RAW y R2 (4k7) a GND en el lado IBUS_RX.
- **Buzzer**: BUZZER_PIN → R3 (1k) → BUZZER_GATE (base de Q1); Q1 colector → BUZZER_LOW; diodo D1 en paralelo con el buzzer.


## Al terminar

1. Rellena el plano de GND (`B`).
2. `Inspeccionar → Reglas de diseño` debe dar **0 violaciones** y **0 elementos sin conectar**.
3. `Archivo → Fabricación → Gerbers` para mandar a fabricar.

