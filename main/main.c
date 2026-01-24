#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_state.h"
#include "uptime_task.h"
#include "mqtt/radio_mqtt_bridge.h"
#include "radio/radio_tx.h"
#include "radio/radio_task.h"
#include "radio/radio_queue.h"
#include "mqtt/mqtt_client_app.h"
#include <nvs.h>
#include <nvs_flash.h>

/* прототипы других функций */
void wifi_init_sta(void);
void oled_task(void *arg);

void app_main(void)
{
    /* 1. NVS (обязательно для Wi-Fi / MQTT) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    /* 2. Системное состояние */
    system_state_init();

    /* 3. Wi-Fi */
    wifi_init_sta();

    /* 4. MQTT */
    // mqtt_app_start();

    /* 5. Очереди радио */
    radio_queue_init();
    radio_tx_init();

    /* 6. Радио RX/TX задачи */
    xTaskCreate(
        radio_task,
        "radio_task",
        4096,
        NULL,
        10,
        NULL
    );

    xTaskCreate(
        radio_tx_task,
        "radio_tx",
        4096,
        NULL,
        9,
        NULL
    );

    /* 7. Bridge Radio → MQTT */
    xTaskCreate(
        radio_mqtt_bridge_task,
        "radio_mqtt_bridge",
        4096,
        NULL,
        5,
        NULL
    );

    /* 8. OLED / uptime / debug (если есть) */
    xTaskCreate(
        uptime_task,
        "uptime",
        2048,
        NULL,
        1,
        NULL
    );
    /* 9. OLED */
    xTaskCreate(oled_task, "oled", 4096, NULL, 1, NULL);
}
