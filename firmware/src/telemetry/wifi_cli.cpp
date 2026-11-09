#include "telemetry/wifi_cli.h"
#include "telemetry/cli.h"
#include "config.h"

#include <WiFi.h>

static WiFiServer server(CLI_TCP_PORT);
static WiFiClient client;
static bool enabled = true;
static char line[96];
static uint8_t line_len = 0;

void wifi_set_enabled(bool enable) {
    enabled = enable;
    if (!enable) {
        client.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
    }
}

void wifi_cli_init(void) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    server.begin();
    server.setNoDelay(true);
}

static void handle_line(const char *l) {
    cli_process_line(l, client);
    client.println();   // separador tras la respuesta
}

void wifi_cli_tick(void) {
    if (!enabled) return;

    if (!client || !client.connected()) {
        client = server.accept();
        line_len = 0;
        if (!client) return;
        client.println("Drone-2026 CLI. Escribe 'help'.");
        client.print("> ");
    }

    while (client.available() > 0) {
        const char c = (char)client.read();
        if (c == '\n' || c == '\r') {
            if (line_len > 0) {
                line[line_len] = '\0';
                handle_line(line);
                line_len = 0;
                client.print("> ");
            }
        } else if (line_len < sizeof(line) - 1) {
            line[line_len++] = c;
        }
    }
}
