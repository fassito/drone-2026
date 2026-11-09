#include "safety/indicators.h"
#include "config.h"

#include <Arduino.h>

static indicator_event_t current = IND_NONE;
static uint8_t beep_index = 0;
static uint8_t beep_count = 0;
static uint32_t last_toggle = 0;
static bool beep_on = false;

static void start_pattern(indicator_event_t evt) {
    current = evt;
    beep_index = 0;
    beep_count = 0;
    beep_on = false;
    last_toggle = millis();
}

void indicators_init(void) {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void indicators_event(indicator_event_t evt) {
    start_pattern(evt);
}

void indicators_tick(void) {
    const uint32_t now = millis();

    switch (current) {
        case IND_NONE:
            return;
        case IND_ARMED: {
            // 3 pitidos cortos (200 ms on, 150 ms off).
            const uint32_t on_ms = 200, off_ms = 150;
            if (beep_on) {
                if (now - last_toggle >= on_ms) {
                    digitalWrite(BUZZER_PIN, LOW);
                    beep_on = false;
                    beep_count++;
                    last_toggle = now;
                    if (beep_count >= 3) current = IND_NONE;
                }
            } else {
                if (now - last_toggle >= off_ms) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    beep_on = true;
                    last_toggle = now;
                }
            }
            break;
        }
        case IND_DISARMED: {
            if (!beep_on) {
                digitalWrite(BUZZER_PIN, HIGH);
                beep_on = true;
                last_toggle = now;
            } else if (now - last_toggle >= 600) {
                digitalWrite(BUZZER_PIN, LOW);
                current = IND_NONE;
            }
            break;
        }
        case IND_FAILSAFE: {
            // 150 ms on / 150 ms off continuo.
            if (now - last_toggle >= 150) {
                digitalWrite(BUZZER_PIN, beep_on ? LOW : HIGH);
                beep_on = !beep_on;
                last_toggle = now;
            }
            break;
        }
        case IND_BATTERY: {
            // 2 pitidos (150/100) + pausa 600 ms.
            const uint32_t period = beep_count % 2 == 0 ? 150 : 100;
            if (beep_on) {
                if (now - last_toggle >= period) {
                    digitalWrite(BUZZER_PIN, LOW);
                    beep_on = false;
                    beep_count++;
                    last_toggle = now;
                }
            } else {
                const uint32_t pause = (beep_count >= 2) ? 600 : 100;
                if (now - last_toggle >= pause) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    beep_on = true;
                    if (beep_count >= 2) beep_count = 0;
                    last_toggle = now;
                }
            }
            break;
        }
    }
}
