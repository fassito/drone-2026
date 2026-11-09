#pragma once

#include <stdbool.h>

// ============================================================
// Interfaz de línea de comandos por WiFi (TCP) para diagnóstico,
// pruebas y telemetría. Solo en tierra: el AP se desactiva al armar.
// Responsable: wifi-cli-engineer. Skill: wifi-cli.
// ============================================================

void wifi_cli_init(void);

// Procesa clientes conectados (no bloqueante). Llamar desde el core 1.
void wifi_cli_tick(void);

// Activa/desactiva el AP WiFi (al armar se desactiva automáticamente).
void wifi_set_enabled(bool enable);
