#include "ssd1306.h"
#include "system_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void oled_task(void *arg)
{
    ssd1306_init();
    char line[32];

    while (1) {
        ssd1306_clear();

        snprintf(line, sizeof(line), "WiFi: %s",
                 g_state.wifi_connected ? "OK" : "ERR");
        ssd1306_draw_string(0, 0, line);

        if (g_state.wifi_connected)
            ssd1306_draw_string(0, 12, g_state.ip);

        snprintf(line, sizeof(line), "MQTT: %s",
                 g_state.mqtt_connected ? "OK" : "ERR");
        ssd1306_draw_string(0, 24, line);

        snprintf(line, sizeof(line), "Uptime: %lus", g_state.uptime_sec);
        ssd1306_draw_string(0, 36, line);

        ssd1306_update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
