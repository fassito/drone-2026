#pragma once

// ============================================================
// Pantalla OLED I2C (~1 pulgada, SSD1306/SH1106).
// Responsable: display-engineer. Skill: oled-display.
// ============================================================

// STUB — pendiente de implementar.

typedef enum {
    OLED_VIEW_STATUS = 0,
    OLED_VIEW_ATTITUDE,
    OLED_VIEW_BARO,
    OLED_VIEW_GPS,
    OLED_VIEW_OBSTACLE,
    OLED_VIEW_COUNT
} oled_view_t;

void oled_init(void);

// Refresco lento (5–10 Hz) desde una tarea RTOS separada; nunca en el
// lazo de control.
void oled_render(oled_view_t view);

// Alterna la vista activa.
void oled_set_view(oled_view_t view);
oled_view_t oled_get_view(void);
