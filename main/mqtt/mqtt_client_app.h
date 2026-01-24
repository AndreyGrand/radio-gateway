#pragma once
#include "mqtt_client.h"
void mqtt_app_start(void);
void mqtt_app_stop(void);
void mqtt_publish(const char *topic,
                  const char *payload);
extern esp_mqtt_client_handle_t g_mqtt;