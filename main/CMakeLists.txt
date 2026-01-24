idf_component_register(
    SRCS "radio/nrf24.c" 
        "radio/crc32.c" 
        "radio/nrf24.c" 
        "radio/radio_task.c"
        "radio/radio_queue.c"
        "radio/radio_tx.c"
        "mqtt/radio_mqtt_bridge.c"
        "mqtt/mqtt_client_app.c"
        "main.c"
        "wifi.c"
        "uptime_task.c"
        "system_state.c"
        "oled_task.c"
        "ssd1306.c"
       
    INCLUDE_DIRS "."
    REQUIRES "freertos" "esp_wifi" "esp_event" "esp_netif" "nvs_flash" "mqtt" "driver" "cjson"
)
