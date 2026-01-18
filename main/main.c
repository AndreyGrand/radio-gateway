#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_state.h"
#include "uptime_task.h"

/* прототипы других функций */
void wifi_init_sta(void);
void oled_task(void *arg);
void mqtt_app_start(void);

void app_main(void)
{
    wifi_init_sta();

    xTaskCreate(uptime_task, "uptime", 2048, NULL, 1, NULL);
    xTaskCreate(oled_task, "oled", 4096, NULL, 1, NULL);

    mqtt_app_start();
}
