#!/usr/bin/env python3
"""Genera la lista de conexiones (netlist) del dron-2026 desde config.h.

Produce pcb/netlist.csv: una tabla componente/pin/red que sirve de base para
el esquemático KiCad. Fuente de verdad: firmware/src/config.h + docs/conexiones.md.
"""
import csv, os

OUT = os.path.join(os.path.dirname(__file__), "netlist.csv")

# (ref, componente, pin, red) — las redes son nombres lógicos.
ROWS = [
    # --- Alimentación ---
    ("J1",  "ESP32 DevKit V1", "VIN",     "5V"),
    ("J1",  "ESP32 DevKit V1", "3V3",     "3V3"),
    ("J1",  "ESP32 DevKit V1", "GND",     "GND"),
    ("XT1", "XT-60 batería",   "+",       "VBAT"),
    ("XT1", "XT-60 batería",   "-",       "GND"),
    # El INA219 mide la corriente que entra al BEC (electrónica), NO la de los
    # motores: su shunt de 0.1 Ω aguanta ~3.2 A y los ESC tiran decenas de A.
    ("BEC1","BEC 5V",          "IN+",     "BEC_IN"),
    ("BEC1","BEC 5V",          "IN-",     "GND"),
    ("BEC1","BEC 5V",          "OUT+",    "5V"),
    ("BEC1","BEC 5V",          "OUT-",    "GND"),

    # --- Sensores I2C (bus compartido) ---
    ("U2", "BMP388 CJMCU-388", "Vin",     "3V3"),
    ("U2", "BMP388 CJMCU-388", "Gnd",     "GND"),
    ("U2", "BMP388 CJMCU-388", "SCK",     "I2C_SCL"),
    ("U2", "BMP388 CJMCU-388", "SDI",     "I2C_SDA"),
    ("U2", "BMP388 CJMCU-388", "CS",      "3V3"),
    ("U2", "BMP388 CJMCU-388", "SDO",     "GND"),

    ("U3", "INA219",           "VCC",     "3V3"),
    ("U3", "INA219",           "GND",     "GND"),
    ("U3", "INA219",           "SDA",     "I2C_SDA"),
    ("U3", "INA219",           "SCL",     "I2C_SCL"),
    ("U3", "INA219",           "Vin+",    "VBAT"),
    ("U3", "INA219",           "Vin-",    "BEC_IN"),

    ("U4", "OLED SSD1306",     "VCC",     "3V3"),
    ("U4", "OLED SSD1306",     "GND",     "GND"),
    ("U4", "OLED SSD1306",     "SDA",     "I2C_SDA"),
    ("U4", "OLED SSD1306",     "SCL",     "I2C_SCL"),

    # --- IMU SPI ---
    ("U5", "MPU-9250 GY-951",  "VCC",     "3V3"),
    ("U5", "MPU-9250 GY-951",  "GND",     "GND"),
    ("U5", "MPU-9250 GY-951",  "SCL/SCK", "SPI_SCK"),
    ("U5", "MPU-9250 GY-951",  "SDA/MOSI","SPI_MOSI"),
    ("U5", "MPU-9250 GY-951",  "AD0/MISO","SPI_MISO"),
    ("U5", "MPU-9250 GY-951",  "NCS",     "SPI_CS_IMU"),

    # --- GPS ---
    ("U6", "GPS Neo 6M",       "VCC",     "3V3"),
    ("U6", "GPS Neo 6M",       "GND",     "GND"),
    ("U6", "GPS Neo 6M",       "TX",      "GPS_RX"),
    ("U6", "GPS Neo 6M",       "RX",      "GPS_TX"),

    # --- HC-12 ---
    ("U7", "HC-12",            "VCC",     "5V"),
    ("U7", "HC-12",            "GND",     "GND"),
    ("U7", "HC-12",            "TX",      "HC12_RX"),
    ("U7", "HC-12",            "RX",      "HC12_TX"),
    ("U7", "HC-12",            "SET",     "HC12_SET"),

    # --- iBus (con divisor 5V->3.3V) ---
    ("J2", "FS-IA6B iBus",     "VCC",     "5V"),
    ("J2", "FS-IA6B iBus",     "GND",     "GND"),
    ("J2", "FS-IA6B iBus",     "SIG",     "IBUS_RAW"),
    ("R1", "Divisor 2.2k",     "1",       "IBUS_RAW"),
    ("R1", "Divisor 2.2k",     "2",       "IBUS_RX"),
    ("R2", "Divisor 4.7k",     "1",       "IBUS_RX"),
    ("R2", "Divisor 4.7k",     "2",       "GND"),

    # --- Sharp GP2Y0A21 x4 ---
    ("U8", "Sharp FR",         "VCC",     "5V"),
    ("U8", "Sharp FR",         "GND",     "GND"),
    ("U8", "Sharp FR",         "Vo",      "SHARP_FR"),
    ("U9", "Sharp R",          "VCC",     "5V"),
    ("U9", "Sharp R",          "GND",     "GND"),
    ("U9", "Sharp R",          "Vo",      "SHARP_R"),
    ("U10","Sharp B",          "VCC",     "5V"),
    ("U10","Sharp B",          "GND",     "GND"),
    ("U10","Sharp B",          "Vo",      "SHARP_B"),
    ("U11","Sharp L",          "VCC",     "5V"),
    ("U11","Sharp L",          "GND",     "GND"),
    ("U11","Sharp L",          "Vo",      "SHARP_L"),

    # --- Motores (salidas OneShot125). Alimentados directo de VBAT (PDB),
    #     no pasan por el shunt del INA219. ---
    ("ESC1","ESC 1",           "V+",      "VBAT"),
    ("ESC1","ESC 1",           "GND",     "GND"),
    ("ESC1","ESC 1",           "SIG",     "MOTOR1"),
    ("ESC2","ESC 2",           "V+",      "VBAT"),
    ("ESC2","ESC 2",           "GND",     "GND"),
    ("ESC2","ESC 2",           "SIG",     "MOTOR2"),
    ("ESC3","ESC 3",           "V+",      "VBAT"),
    ("ESC3","ESC 3",           "GND",     "GND"),
    ("ESC3","ESC 3",           "SIG",     "MOTOR3"),
    ("ESC4","ESC 4",           "V+",      "VBAT"),
    ("ESC4","ESC 4",           "GND",     "GND"),
    ("ESC4","ESC 4",           "SIG",     "MOTOR4"),

    # --- Buzzer (driver NPN) ---
    ("Q1", "NPN 2N2222",       "B",       "BUZZER_GATE"),
    ("Q1", "NPN 2N2222",       "E",       "GND"),
    ("Q1", "NPN 2N2222",       "C",       "BUZZER_LOW"),
    ("R3", "Base 1k",          "1",       "BUZZER_PIN"),
    ("R3", "Base 1k",          "2",       "BUZZER_GATE"),
    ("LS1","Buzzer 5V",        "+",       "5V"),
    ("LS1","Buzzer 5V",        "-",       "BUZZER_LOW"),
    ("D1", "Flyback 1N4148",   "A",       "BUZZER_LOW"),
    ("D1", "Flyback 1N4148",   "K",       "5V"),

    # --- LED ---
    ("D2", "LED estado",       "A",       "LED_PIN"),
    ("D2", "LED estado",       "K",       "GND"),

    # --- GPIO del ESP32 (referencias de red) ---
    ("J1", "ESP32 DevKit V1",  "IO2",     "LED_PIN"),
    ("J1", "ESP32 DevKit V1",  "IO4",     "GPS_TX"),
    ("J1", "ESP32 DevKit V1",  "IO5",     "SPI_CS_IMU"),
    ("J1", "ESP32 DevKit V1",  "IO13",    "HC12_SET"),
    ("J1", "ESP32 DevKit V1",  "IO16",    "HC12_RX"),
    ("J1", "ESP32 DevKit V1",  "IO17",    "HC12_TX"),
    ("J1", "ESP32 DevKit V1",  "IO18",    "SPI_SCK"),
    ("J1", "ESP32 DevKit V1",  "IO19",    "SPI_MISO"),
    ("J1", "ESP32 DevKit V1",  "IO21",    "I2C_SDA"),
    ("J1", "ESP32 DevKit V1",  "IO22",    "I2C_SCL"),
    ("J1", "ESP32 DevKit V1",  "IO23",    "SPI_MOSI"),
    ("J1", "ESP32 DevKit V1",  "IO25",    "MOTOR1"),
    ("J1", "ESP32 DevKit V1",  "IO26",    "MOTOR2"),
    ("J1", "ESP32 DevKit V1",  "IO27",    "BUZZER_PIN"),
    ("J1", "ESP32 DevKit V1",  "IO32",    "MOTOR3"),
    ("J1", "ESP32 DevKit V1",  "IO33",    "MOTOR4"),
    ("J1", "ESP32 DevKit V1",  "IO34",    "IBUS_RX"),
    ("J1", "ESP32 DevKit V1",  "IO35",    "GPS_RX"),
    ("J1", "ESP32 DevKit V1",  "IO36",    "SHARP_FR"),
    ("J1", "ESP32 DevKit V1",  "IO37",    "SHARP_R"),
    ("J1", "ESP32 DevKit V1",  "IO38",    "SHARP_B"),
    ("J1", "ESP32 DevKit V1",  "IO39",    "SHARP_L"),
]

with open(OUT, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["ref", "componente", "pin", "red"])
    w.writerows(ROWS)

print(f"OK: {len(ROWS)} conexiones -> {OUT}")
