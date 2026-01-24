#include "ssd1306.h"
#include "system_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "mqtt/mqtt_client_app.h"

void oled_task(void *arg)
{
    ssd1306_init();
    char line[32];

    while (1) {
        ssd1306_clear();

        // Text rotated 90 degrees right, spread across full 64-pixel height
        
        // Top: WiFi status (y=0, yellow section)
        snprintf(line, sizeof(line), "WiFi: %s",
                 g_state.wifi_connected ? "OK" : "ERR");
        ssd1306_draw_string(0, 0, line);

        // Middle: IP address (y=16, middle blue section)
        if (g_state.wifi_connected)
            snprintf(line, sizeof(line), "%s", g_state.ip);
        else
            snprintf(line, sizeof(line), "Connecting");
        ssd1306_draw_string(0, 16, line);

        // Bottom: MQTT status (y=32, lower blue section)
        snprintf(line, sizeof(line), "MQTT: %s",
                 g_state.mqtt_connected ? "OK" : "ERR");
        ssd1306_draw_string(0, 32, line);
        // Bottom-most: Radio RX count (y=48, bottom blue section)
        snprintf(line, sizeof(line), "RX: %lu", g_state.radio_rx);
        ssd1306_draw_string(0, 48, line);
        snprintf(line, sizeof(line), "UT: %lu", g_state.uptime_sec);
        ssd1306_draw_string(60, 48, line);

        ssd1306_update();
        vTaskDelay(pdMS_TO_TICKS(1000));
        mqtt_publish("gateway/status", line);

    }
}
