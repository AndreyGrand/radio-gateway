idf_component_register(
    SRCS "main.c"
         "wifi.c"
         "uptime_task.c"
         "system_state.c"
         "oled_task.c"
         "ssd1306.c"
         "mqtt_client_app.c"
    INCLUDE_DIRS "."
    REQUIRES "freertos" "esp_wifi" "esp_event" "esp_netif" "nvs_flash" "mqtt" "driver"
)
