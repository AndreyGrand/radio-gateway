#include "mqtt_client.h"
#include "esp_log.h"
#include "system_state.h"

static const char *TAG = "mqtt";

/* forward declaration */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data);

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = CONFIG_MQTT_URI,
        .credentials.username = CONFIG_MQTT_USER,
        .credentials.authentication.password = CONFIG_MQTT_PASS,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(client);
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected");
        g_state.mqtt_connected = true;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT Disconnected");
        g_state.mqtt_connected = false;
        break;
    default:
        break;
    }
}
