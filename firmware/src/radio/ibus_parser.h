#pragma once

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Parser iBus puro (sin hardware): valida y decodifica una trama.
// ============================================================

#define IBUS_FRAME_LEN 32
#define IBUS_CHANNELS 6

// Decodifica una trama iBus de 32 bytes (0x20 0x40 + 30 datos + checksum).
// Los canales se escalan a [-1, +1].
// Devuelve true si la trama es válida.
bool ibus_parse_frame(const uint8_t *frame, uint32_t len, float channels[IBUS_CHANNELS]);
