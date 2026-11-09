#pragma once

#include <stddef.h>
#include <stdbool.h>

// ============================================================
// Radio de telemetría HC-12 (433/868/915 MHz): enlace
// bidireccional en vuelo. Stream transparente que reutiliza el
// parser de la WiFi CLI.
// Responsable: hc12-link-engineer. Skill: hc12-telemetry.
// ============================================================

// STUB — pendiente de implementar.

void hc12_init(void);

// Stream no bloqueante para la CLI (mismo parser que WiFi).
bool hc12_available(void);
int  hc12_read_byte(void);
void hc12_write(const char *data, size_t len);

// Modo AT: pone el pin SET en HIGH/LOW para configurar el módulo.
void hc12_set_at_mode(bool enable);
