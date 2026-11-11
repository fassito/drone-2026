#!/usr/bin/env python3
"""Genera drone-2026.kicad_sch completo desde la netlist.

Incrusta las definiciones de símbolo (no depende del buscador de bibliotecas)
y conecta cada pin con una etiqueta global, de modo que la netlist resultante
coincide con netlist.csv.
"""
import os
import re
import uuid

SYM_DIR = ("/home/fass/.local/share/flatpak/runtime/"
           "org.kicad.KiCad.Library.Symbols/x86_64/stable/"
           "129cbdeb4de6e0769586666d4ade53cc26057337479431da6a8b11bc5297182e"
           "/files/symbols")
PROJ = "drone-2026"
OUT = os.path.join(os.path.dirname(__file__), PROJ + ".kicad_sch")

STUB = 5.08       # longitud del hilo desde el pin a la etiqueta
COL_W, ROW_H = 90.0, 100.0


# ---------------------------------------------------------------- utilidades

def find_block(text, i0):
    """Devuelve el bloque balanceado que empieza en i0."""
    depth, i = 0, i0
    while i < len(text):
        if text[i] == '(':
            depth += 1
        elif text[i] == ')':
            depth -= 1
            if depth == 0:
                return text[i0:i + 1], i + 1
        i += 1
    return text[i0:], len(text)


def load_symbol(lib, name):
    """Extrae el bloque (symbol "name" ...) de la biblioteca."""
    path = os.path.join(SYM_DIR, lib + ".kicad_sym")
    src = open(path, encoding="utf-8").read()
    idx = src.find('(symbol "%s"' % name)
    if idx < 0:
        raise SystemExit("símbolo no encontrado: %s:%s" % (lib, name))
    return find_block(src, idx)[0]


def parse_pins(block):
    """Lista de (numero, nombre, x, y, angulo)."""
    pins = []
    for m in re.finditer(
            r'\(pin \w+ \w+\s*\(at ([-\d.]+) ([-\d.]+) (\d+)\)'
            r'.*?\(name "([^"]*)"', block, re.S):
        x, y, ang, nm = float(m.group(1)), float(m.group(2)), int(m.group(3)), m.group(4)
        # el número aparece después del nombre
        tail = block[m.end():m.end() + 400]
        num = re.search(r'\(number "([^"]*)"', tail)
        pins.append((num.group(1) if num else "", nm, x, y, ang))
    return pins


def embed(lib, name, block):
    """Renombra a 'Lib:Name' y elimina claves no válidas en lib_symbols."""
    first = block.replace('(symbol "%s"' % name, '(symbol "%s:%s"' % (lib, name), 1)
    # claves presentes en las librerías pero no admitidas en el esquemático
    for key in ("duplicate_pin_numbers_are_jumpers", "in_pos_files"):
        first = re.sub(r'\n\s*\(%s \w+\)' % key, "", first)
    return first


# ------------------------------------------------------- mapa de conexiones

# J1 ESP32 DevKit V1 (30 pines, 2x15 odd/even).
# OJO: este pinout debe verificarse contra la placa física antes de fabricar.
ESP32_PINOUT = {
    # lado impar (columna izquierda, arriba->abajo)
    "1": None,        # EN
    "3": "SHARP_FR",  # VP / IO36
    "5": "SHARP_L",   # VN / IO39
    "7": "IBUS_RX",   # IO34
    "9": "GPS_RX",    # IO35
    "11": "MOTOR3",   # IO32
    "13": "MOTOR4",   # IO33
    "15": "MOTOR1",   # IO25
    "17": "MOTOR2",   # IO26
    "19": "BUZZER_PIN",  # IO27
    "21": None,       # IO14
    "23": None,       # IO12
    "25": "HC12_SET", # IO13
    "27": "GND",
    "29": "5V",       # VIN
    # lado par (columna derecha, arriba->abajo)
    "2": "3V3",
    "4": "GND",
    "6": None,        # IO15
    "8": "LED_PIN",   # IO2
    "10": "GPS_TX",   # IO4
    "12": "HC12_RX",  # IO16
    "14": "HC12_TX",  # IO17
    "16": "SPI_CS_IMU",  # IO5
    "18": "SPI_SCK",  # IO18
    "20": "SPI_MISO",  # IO19
    "22": "I2C_SDA",  # IO21
    "24": "I2C_SCL",  # IO22
    "26": "SPI_MOSI",  # IO23
    "28": "GND",
    "30": "3V3",
}

# (ref, valor, lib, simbolo, columna, fila, {pin: red})
COMPONENTS = [
    ("J1", "ESP32-DevKit-V1", "Connector_Generic", "Conn_02x15_Odd_Even", 0, 0, ESP32_PINOUT),

    ("U2", "BMP388", "Connector_Generic", "Conn_01x08", 1, 0, {
        "1": "3V3", "2": None, "3": "GND", "4": "I2C_SCL",
        "5": "GND", "6": "I2C_SDA", "7": "3V3", "8": None}),
    ("U3", "INA219", "Connector_Generic", "Conn_01x06", 2, 0, {
        "1": "3V3", "2": "GND", "3": "I2C_SCL", "4": "I2C_SDA",
        "5": "VBAT", "6": "BEC_IN"}),
    ("U4", "OLED-SSD1306", "Connector_Generic", "Conn_01x04", 3, 0, {
        "1": "3V3", "2": "GND", "3": "I2C_SCL", "4": "I2C_SDA"}),

    ("U5", "MPU-9250-GY951", "Connector_Generic", "Conn_01x10", 0, 1, {
        "1": "3V3", "2": "GND", "3": "SPI_SCK", "4": "SPI_MOSI",
        "5": None, "6": None, "7": "SPI_MISO", "8": None,
        "9": "SPI_CS_IMU", "10": None}),
    ("U6", "GPS-NEO6M", "Connector_Generic", "Conn_01x04", 1, 1, {
        "1": "3V3", "2": "GND", "3": "GPS_TX", "4": "GPS_RX"}),
    ("U7", "HC-12", "Connector_Generic", "Conn_01x05", 2, 1, {
        "1": "5V", "2": "GND", "3": "HC12_TX", "4": "HC12_RX", "5": "HC12_SET"}),

    ("U8", "SHARP-FR", "Connector_Generic", "Conn_01x03", 0, 2, {
        "1": "5V", "2": "GND", "3": "SHARP_FR"}),
    ("U9", "SHARP-R", "Connector_Generic", "Conn_01x03", 1, 2, {
        "1": "5V", "2": "GND", "3": None}),
    ("U10", "SHARP-B", "Connector_Generic", "Conn_01x03", 2, 2, {
        "1": "5V", "2": "GND", "3": None}),
    ("U11", "SHARP-L", "Connector_Generic", "Conn_01x03", 3, 2, {
        "1": "5V", "2": "GND", "3": "SHARP_L"}),

    ("ESC1", "ESC-FR", "Connector_Generic", "Conn_01x03", 0, 3, {
        "1": "GND", "2": "VBAT", "3": "MOTOR1"}),
    ("ESC2", "ESC-RL", "Connector_Generic", "Conn_01x03", 1, 3, {
        "1": "GND", "2": "VBAT", "3": "MOTOR2"}),
    ("ESC3", "ESC-FL", "Connector_Generic", "Conn_01x03", 2, 3, {
        "1": "GND", "2": "VBAT", "3": "MOTOR3"}),
    ("ESC4", "ESC-RR", "Connector_Generic", "Conn_01x03", 3, 3, {
        "1": "GND", "2": "VBAT", "3": "MOTOR4"}),

    ("J2", "FS-IA6B-iBus", "Connector_Generic", "Conn_01x03", 0, 4, {
        "1": "5V", "2": "GND", "3": "IBUS_RAW"}),
    ("BEC1", "BEC-5V", "Connector_Generic", "Conn_01x04", 1, 4, {
        "1": "BEC_IN", "2": "GND", "3": "5V", "4": "GND"}),
    ("XT1", "XT60-BATERIA", "Connector_Generic", "Conn_01x02", 2, 4, {
        "1": "VBAT", "2": "GND"}),

    ("R1", "2k2", "Device", "R", 3, 4, {"1": "IBUS_RAW", "2": "IBUS_RX"}),
    ("R2", "4k7", "Device", "R", 3, 4, {"1": "IBUS_RX", "2": "GND"}),
    ("R3", "1k", "Device", "R", 3, 5, {"1": "BUZZER_PIN", "2": "BUZZER_GATE"}),
    ("R4", "4k7", "Device", "R", 1, 5, {"1": "I2C_SDA", "2": "3V3"}),
    ("R5", "4k7", "Device", "R", 2, 5, {"1": "I2C_SCL", "2": "3V3"}),

    ("Q1", "2N2222A", "Transistor_BJT", "Q_NPN_BEC", 0, 5, {
        "B": "BUZZER_GATE", "C": "BUZZER_LOW", "E": "GND"}),
    ("LS1", "BUZZER-5V", "Device", "Buzzer", 0, 6, {
        "1": "5V", "2": "BUZZER_LOW"}),
    ("D1", "1N4148", "Device", "D", 1, 6, {"K": "5V", "A": "BUZZER_LOW"}),
    ("D2", "LED-ESTADO", "Device", "LED", 2, 6, {"K": "GND", "A": "LED_PIN"}),

    ("C1", "100nF", "Device", "C", 3, 6, {"1": "3V3", "2": "GND"}),
    ("C2", "100nF", "Device", "C", 0, 7, {"1": "3V3", "2": "GND"}),
    ("C3", "100nF", "Device", "C", 1, 7, {"1": "3V3", "2": "GND"}),
    ("C4", "100nF", "Device", "C", 2, 7, {"1": "5V", "2": "GND"}),
    ("C5", "100nF", "Device", "C", 3, 7, {"1": "5V", "2": "GND"}),
    ("C6", "100uF", "Device", "C", 0, 8, {"1": "5V", "2": "GND"}),
]


def main():
    lib_cache = {}
    body = []
    top = []
    used_nets = set()

    for ref, value, lib, sym, col, row, netmap in COMPONENTS:
        key = (lib, sym)
        if key not in lib_cache:
            lib_cache[key] = load_symbol(lib, sym)
        raw = lib_cache[key]
        pins = parse_pins(raw)

        cx = 60.0 + col * COL_W
        cy = 60.0 + row * ROW_H
        uid = str(uuid.uuid4())

        body.append("\t(symbol\n\t\t(lib_id \"%s:%s\")\n\t\t(at %.2f %.2f 0)\n"
                    "\t\t(unit 1)\n\t\t(body_style 1)\n\t\t(exclude_from_sim no)\n"
                    "\t\t(in_bom yes)\n\t\t(on_board yes)\n\t\t(in_pos_files yes)\n"
                    "\t\t(dnp no)\n\t\t(uuid \"%s\")" % (lib, sym, cx, cy, uid))
        body.append('\t\t(property "Reference" "%s" (at %.2f %.2f 0)'
                    ' (effects (font (size 1.27 1.27)) (justify left)))'
                    % (ref, cx, cy - 2))
        body.append('\t\t(property "Value" "%s" (at %.2f %.2f 0)'
                    ' (effects (font (size 1.27 1.27)) (justify left)))'
                    % (value, cx, cy + 2))
        body.append('\t\t(property "Footprint" "" (at %.2f %.2f 0)'
                    ' (effects (font (size 1.27 1.27)) (hide yes)))' % (cx, cy))
        body.append('\t\t(property "Datasheet" "" (at %.2f %.2f 0)'
                    ' (effects (font (size 1.27 1.27)) (hide yes)))' % (cx, cy))

        for num, nm, px, py, ang in pins:
            body.append('\t\t(pin "%s" (uuid "%s"))' % (num, uuid.uuid4()))

            net = netmap.get(num, netmap.get(nm))
            if net is None:
                # marca de "sin conectar" para que el ERC quede limpio
                top.append('\t(no_connect (at %.2f %.2f) (uuid "%s"))'
                           % (cx + px, cy - py, uuid.uuid4()))
                continue
            used_nets.add(net)

            gx, gy = cx + px, cy - py
            # dirección hacia fuera (opuesta a la del pin)
            if ang == 0:
                dx, dy, lrot = -STUB, 0.0, 180
            elif ang == 180:
                dx, dy, lrot = STUB, 0.0, 0
            elif ang == 90:
                # pin apunta hacia arriba (lib) -> fuera = abajo (esquema +Y)
                dx, dy, lrot = 0.0, STUB, 0
            else:
                # 270: pin apunta hacia abajo (lib) -> fuera = arriba
                dx, dy, lrot = 0.0, -STUB, 0
            ex, ey = gx + dx, gy + dy

            # hilos y etiquetas van al nivel superior, FUERA del símbolo
            top.append('\t(wire (pts (xy %.2f %.2f) (xy %.2f %.2f))'
                       ' (stroke (width 0) (type default)) (uuid "%s"))'
                       % (gx, gy, ex, ey, uuid.uuid4()))
            top.append(
                '\t(global_label "%s"\n\t\t(shape input)\n\t\t(at %.2f %.2f %d)\n'
                '\t\t(fields_autoplaced yes)\n\t\t(effects (font (size 1.27 1.27)))\n'
                '\t\t(uuid "%s")\n'
                '\t\t(property "Intersheetrefs" "${INTERSHEET_REFS}" (at %.2f %.2f 0)\n'
                '\t\t\t(effects (font (size 1.27 1.27)) (hide yes))\n\t\t)\n\t)'
                % (net, ex, ey, lrot, uuid.uuid4(), ex, ey))

        body.append('\t\t(instances\n\t\t\t(project "%s"\n\t\t\t\t(path "/%s"'
                    ' (reference "%s") (unit 1))\n\t\t\t)\n\t\t)' % (PROJ, SHEET_UUID, ref))
        body.append("\t)")

    # lib_symbols
    libsyms = []
    for (lib, sym), raw in lib_cache.items():
        libsyms.append("\t\t" + embed(lib, sym, raw).replace("\n", "\n\t"))

    sch = '''(kicad_sch
	(version 20260306)
	(generator "eeschema")
	(generator_version "10.0")
	(uuid "%s")
	(paper "A2")
	(title_block
		(title "Drone-2026")
		(date "2026-11-11")
		(rev "1.0")
		(company "fassito")
	)
	(lib_symbols
%s
	)
%s
	(sheet_instances
		(path "/" (page "1"))
	)
	(embedded_fonts no)
)
''' % (SHEET_UUID, "\n".join(libsyms), "\n".join(body + top))

    open(OUT, "w", encoding="utf-8").write(sch)
    print("OK -> %s" % OUT)
    print("componentes: %d   redes: %d" % (len(COMPONENTS), len(used_nets)))


SHEET_UUID = "a1b2c3d4-1111-4222-8333-444455556666"

if __name__ == "__main__":
    main()
