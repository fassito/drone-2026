#include "telemetry/ota.h"
#include "config.h"

#include <ArduinoOTA.h>

void ota_init(void) {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword("");   // opcional: fijar una clave

    ArduinoOTA.onStart([]() {});
    ArduinoOTA.onEnd([]() {});
    ArduinoOTA.onError([](ota_error_t error) { (void)error; });

    ArduinoOTA.begin();
}

void ota_handle(void) {
    ArduinoOTA.handle();
}
