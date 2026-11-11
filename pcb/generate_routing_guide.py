#!/usr/bin/env python3
"""Genera GUIA_ENRUTADO.md con las conexiones pendientes y sus coordenadas.

  flatpak run --command=python3 org.kicad.KiCad pcb/generate_routing_guide.py
"""
import os
from collections import defaultdict
import pcbnew

HERE = os.path.dirname(os.path.abspath(__file__))
PCB = os.path.join(HERE, "drone-2026.kicad_pcb")
OUT = os.path.join(HERE, "GUIA_ENRUTADO.md")

# orden de enrutado y anchura recomendada (mm)
ORDER = [
    ("VBAT", 1.5, "Potencia: batería → ESC. Lo más corto y ancho posible."),
    ("5V", 0.8, "Alimentación de la electrónica desde el BEC."),
    ("3V3", 0.8, "Alimentación de sensores desde el regulador del DevKit."),
    ("I2C_SDA", 0.3, "Rutear junto a I2C_SCL, en paralelo."),
    ("I2C_SCL", 0.3, "Rutear junto a I2C_SDA, en paralelo."),
    ("SPI_SCK", 0.3, "Bus SPI de la IMU: mantener corto."),
    ("SPI_MOSI", 0.3, "Bus SPI de la IMU."),
    ("SPI_MISO", 0.3, "Bus SPI de la IMU."),
    ("SPI_CS_IMU", 0.3, "Bus SPI de la IMU."),
    ("IBUS_RAW", 0.3, "Señal del receptor al divisor."),
    ("IBUS_RX", 0.3, "Salida del divisor al GPIO34."),
    ("MOTOR1", 0.3, "Señal OneShot125 al ESC."),
    ("MOTOR2", 0.3, "Señal OneShot125 al ESC."),
    ("MOTOR3", 0.3, "Señal OneShot125 al ESC."),
    ("MOTOR4", 0.3, "Señal OneShot125 al ESC."),
    ("GPS_TX", 0.3, "UART del GPS."),
    ("GPS_RX", 0.3, "UART del GPS."),
    ("HC12_TX", 0.3, "UART2 del HC-12."),
    ("HC12_RX", 0.3, "UART2 del HC-12."),
    ("HC12_SET", 0.3, "Modo AT del HC-12."),
    ("SHARP_FR", 0.3, "Analógica del sensor frontal."),
    ("SHARP_R", 0.3, "Analógica del sensor derecho."),
    ("SHARP_B", 0.3, "Analógica del sensor trasero."),
    ("SHARP_L", 0.3, "Analógica del sensor izquierdo."),
    ("LED_PIN", 0.3, "LED de estado."),
    ("BUZZER_PIN", 0.3, "GPIO del buzzer a la base del transistor."),
    ("BUZZER_GATE", 0.3, "Base del transistor a la resistencia."),
    ("BUZZER_LOW", 0.3, "Colector del transistor al buzzer y al diodo."),
    ("BEC_IN", 1.0, "Entrada del BEC, a través del shunt del INA219."),
]


def nearest_order(pts):
    """Encadena los pads por cercanía para que la ruta fluya."""
    rest = list(pts)
    out = [rest.pop(0)]
    while rest:
        last = out[-1]
        k = min(range(len(rest)),
                key=lambda i: (rest[i][2] - last[2]) ** 2 + (rest[i][3] - last[3]) ** 2)
        out.append(rest.pop(k))
    return out


def main():
    board = pcbnew.LoadBoard(PCB)
    pads = defaultdict(list)
    for fp in board.GetFootprints():
        for p in fp.Pads():
            n = p.GetNetname()
            if not n or n == "GND":
                continue
            pos = p.GetPosition()
            pads[n].append((fp.GetReference(), p.GetNumber(),
                            pcbnew.ToMM(pos.x), pcbnew.ToMM(pos.y)))

    xs, ys = [], []
    for d in board.GetDrawings():
        if d.GetLayer() == pcbnew.Edge_Cuts:
            bb = d.GetBoundingBox()
            xs += [pcbnew.ToMM(bb.GetLeft()), pcbnew.ToMM(bb.GetRight())]
            ys += [pcbnew.ToMM(bb.GetTop()), pcbnew.ToMM(bb.GetBottom())]

    lines = []
    lines.append("# Guía de enrutado — placa Drone-2026\n")
    lines.append("Placa de **%.0f × %.0f mm**, 2 capas (`F.Cu` / `B.Cu`). "
                 "Los componentes ya están colocados y las redes asignadas: solo "
                 "faltan las pistas.\n" % (max(xs) - min(xs), max(ys) - min(ys)))
    lines.append("Coordenadas en mm con el origen en la esquina superior "
                 "izquierda (Y crece hacia abajo, como en KiCad).\n")
    lines.append("\n## Antes de empezar\n")
    lines.append("1. **GND no aparece aquí**: se resuelve con un plano de masa. "
                 "En el editor de PCB: `Añadir zona rellena` → capa `B.Cu` → "
                 "red `GND` → dibuja el rectángulo del contorno → `B` para "
                 "rellenar. (También puedes usar `pcb/add_gnd_zone.py`.)")
    lines.append("2. Traza en el orden de esta guía: primero potencia (ancha), "
                 "luego buses y por último señales.")
    lines.append("3. Atajo en KiCad: pulsa `X` para enrutar, `V` para poner una "
                 "vía, `B` para rellenar zonas.")
    lines.append("\n## Redes a conectar\n")

    idx = 0
    for name, width, note in ORDER:
        pts = pads.get(name, [])
        if len(pts) < 2:
            continue
        pts = nearest_order(pts)
        idx += 1
        total = sum(((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5
                    for (_, _, x1, y1), (_, _, x2, y2) in zip(pts, pts[1:]))
        lines.append("### %d. `%s` — pista de %.1f mm  (%.0f mm en total)\n"
                     % (idx, name, width, total))
        lines.append("*%s*\n" % note)
        path = "  →  ".join("%s.%s (%.1f, %.1f)"
                           % (r, pn, x, y) for r, pn, x, y in pts)
        lines.append("%s\n" % path)

    lines.append("\n## Consejos por bloque\n")
    lines.append("- **VBAT**: de XT1 a los 4 ESC y a INA219. Es la de más "
                 "corriente: usa 1.5 mm o más y evita rodeos.")
    lines.append("- **BEC_IN**: pasa por el shunt del INA219 (Vin+ / Vin−). "
                 "Respétalo o la medición de corriente no funciona.")
    lines.append("- **I2C_SDA / SCL**: trázalas en paralelo y cortas; compartidas "
                 "por OLED, BMP388 e INA219.")
    lines.append("- **SPI**: SCK/MOSI/MISO/CS de la IMU al DevKit. Mantenlas "
                 "juntas y lejos de VBAT para evitar ruido.")
    lines.append("- **MOTOR1-4**: solo señal, 0.3 mm. Van del DevKit a cada ESC.")
    lines.append("- **Divisor iBus**: R1 (2k2) en serie desde IBUS_RAW y R2 "
                 "(4k7) a GND en el lado IBUS_RX.")
    lines.append("- **Buzzer**: BUZZER_PIN → R3 (1k) → BUZZER_GATE (base de Q1); "
                 "Q1 colector → BUZZER_LOW; diodo D1 en paralelo con el buzzer.\n")
    lines.append("\n## Al terminar\n")
    lines.append("1. Rellena el plano de GND (`B`).")
    lines.append("2. `Inspeccionar → Reglas de diseño` debe dar **0 violaciones** "
                 "y **0 elementos sin conectar**.")
    lines.append("3. `Archivo → Fabricación → Gerbers` para mandar a fabricar.\n")

    open(OUT, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print("guía escrita -> %s (%d redes)" % (OUT, idx))


if __name__ == "__main__":
    main()
