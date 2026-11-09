#pragma once

#include <Print.h>

// ============================================================
// Parser de comandos compartido entre WiFi (TCP) y HC-12.
// ============================================================

void cli_process_line(const char *line, Print &out);
