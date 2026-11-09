#pragma once

// ============================================================
// Indicadores: buzzer 5V y LED de estado (patrones no bloqueantes).
// ============================================================

typedef enum {
    IND_NONE = 0,
    IND_ARMED,      // 3 pitidos cortos
    IND_DISARMED,   // 1 pitido largo
    IND_FAILSAFE,   // pitidos rápidos continuos
    IND_BATTERY,    // 2 pitidos repetidos
} indicator_event_t;

void indicators_init(void);
void indicators_event(indicator_event_t evt);
void indicators_tick(void);   // llamar desde loop() (núcleo 1)
