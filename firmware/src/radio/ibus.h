#pragma once

#include <stdbool.h>

// ============================================================
// Decodificación iBus del receptor FlySky FS-IA6B (UART 115200).
// Responsable: radio-receiver-engineer. Skill: ibus-integration.
// ============================================================

// STUB — pendiente de implementar.

#define IBUS_CHANNELS 6

typedef struct {
    // Canales escalados a [-1, +1]:
    // [0]=roll, [1]=pitch, [2]=throttle, [3]=yaw, [4]=arm, [5]=mode
    float channel[IBUS_CHANNELS];
    bool link_ok;   // false si no llega trama válida (failsafe de receptor)
} ibus_state_t;

void ibus_init(void);
ibus_state_t ibus_read(void);
