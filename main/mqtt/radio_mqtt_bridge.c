#include "radio_mqtt_bridge.h"
#include "../radio/radio_queue.h"
#include "mqtt_client_app.h"
#include "../radio/rmessage.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cJSON.h>
#include <stdio.h>

static void publish_discovery(uint8_t node,
                              const char *name,
                              const char *unit)
{
    char topic[128];
    char payload[256];

    snprintf(topic, sizeof(topic),
        "homeassistant/sensor/radio_%d_%s/config",
        node, name);

    snprintf(payload, sizeof(payload),
        "{"
        "\"name\":\"Node%d %s\","
        "\"state_topic\":\"radio/%d/state\","
        "\"value_template\":\"{{ value_json.%s }}\","
        "\"unit_of_measurement\":\"%s\","
        "\"unique_id\":\"radio_%d_%s\""
        "}",
        node, name, node, name, unit, node, name);

    mqtt_publish(topic, payload);
}

void radio_mqtt_bridge_task(void *arg)
{
    rmessage_t msg;
    bool discovered[256] = {0};

    while (1) {
        if (!radio_rx_pop(&msg)) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (msg.hdr.type != RM_TYPE_SENSOR)
            continue;

        cJSON *root = cJSON_Parse((char *)msg.data);
        if (!root) continue;

        uint8_t node = msg.hdr.dst;

        cJSON *name  = cJSON_GetObjectItem(root, "name");
        cJSON *value = cJSON_GetObjectItem(root, "value");
        cJSON *unit  = cJSON_GetObjectItem(root, "unit");

        if (!name || !value) {
            cJSON_Delete(root);
            continue;
        }

        if (!discovered[node]) {
            publish_discovery(node,
                              name->valuestring,
                              unit ? unit->valuestring : "");
            discovered[node] = true;
        }

        char topic[64];
        snprintf(topic, sizeof(topic),
                 "radio/%d/state", node);

        mqtt_publish(topic,
                     (char *)msg.data);

        cJSON_Delete(root);
    }
}
