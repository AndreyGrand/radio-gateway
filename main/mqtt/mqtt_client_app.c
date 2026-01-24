#include "mqtt_client.h"
#include "mqtt_client_app.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "system_state.h"
#include "mqtt_client_app.h"

static const char *TAG = "mqtt";
esp_mqtt_client_handle_t g_mqtt = NULL;

/* forward declaration */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data);

void mqtt_app_start(void)
{
    // Защита от повторной инициализации
    if (g_mqtt != NULL) {
        ESP_LOGW(TAG, "MQTT already started, skipping");
        return;
    }

    // Проверка: есть ли IP адрес перед подключением
    if (g_state.ip[0] == '\0') {
        ESP_LOGW(TAG, "No IP address yet, MQTT start deferred");
        return;
    }

    char client_id[32];
    // uint64_t mac;
    uint8_t mac[6];
    esp_read_mac((uint8_t *)&mac, ESP_MAC_WIFI_STA);

    // snprintf(client_id, sizeof(client_id),
    //      "gateway_%04X", (uint16_t)(mac & 0xFFFF));
    snprintf(client_id, sizeof(client_id),
             "gateway_%02X%02X%02X",
             mac[3], mac[4], mac[5]);
    esp_mqtt_client_config_t cfg = {
                .broker = {
            .address = {
                .uri = CONFIG_MQTT_URI,
            },
        },
        .credentials = {
            .username = CONFIG_MQTT_USER,
            .authentication = {
                .password = CONFIG_MQTT_PASS,
            },
            .client_id = client_id,
        },

        .session = {
            .keepalive = 30,
            .protocol_ver = MQTT_PROTOCOL_V_3_1_1,
            .last_will = {
                .topic = "gateway/status",
                .msg = "offline",
                .msg_len = 7,
                .qos = 1,
                .retain = true,
            },
        },

        .network = {
            .reconnect_timeout_ms = 5000,
            .disable_auto_reconnect = false,
            .timeout_ms = 10000,
        },
        .task.stack_size = 4096,  // Increase stack size for MQTT task
        // .broker.address.uri = CONFIG_MQTT_URI,
        // .credentials.username = CONFIG_MQTT_USER,
        // .credentials.authentication.password = CONFIG_MQTT_PASS,
        // /* 🔥 LWT */
        // .session.last_will.topic = "gateway/status",
        // .session.last_will.msg = "offline",
        // .session.last_will.qos = 1,
        // .session.last_will.retain = true,
        // .session.keepalive = 30,  // Send PING every 30 seconds
        // .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        // .network.disable_auto_reconnect = false,  // Enable auto-reconnect
        // .task.stack_size = 4096,  // Increase stack size for MQTT task
        // .credentials.client_id = client_id,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);

    g_mqtt = client;  // Store the handle globally

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    ESP_LOGI(TAG, "Starting MQTT at %s", CONFIG_MQTT_URI);
    esp_mqtt_client_start(client);
}

void mqtt_publish(const char *topic, const char *data)
{
    if (g_mqtt == NULL) {
        ESP_LOGW(TAG, "MQTT client not initialized");
        return;
    }
    esp_mqtt_client_publish(g_mqtt, topic, data, 0, 1, false);
}

void mqtt_app_stop(void)
{
    if (g_mqtt == NULL) {
        ESP_LOGW(TAG, "MQTT not running");
        return;
    }

    ESP_LOGI(TAG, "Stopping MQTT...");
    
    // Отправить LWT перед отключением
    mqtt_publish("gateway/status", "offline");
    
    // Остановить клиент
    esp_mqtt_client_stop(g_mqtt);
    
    // Деинициализировать клиент
    esp_mqtt_client_destroy(g_mqtt);
    
    g_mqtt = NULL;
    g_state.mqtt_connected = false;
    
    ESP_LOGI(TAG, "MQTT stopped");
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
        vTaskDelay(pdMS_TO_TICKS(200));  // 🔥 дать брокеру стабилизироваться
        mqtt_publish("gateway/status", "online");
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
