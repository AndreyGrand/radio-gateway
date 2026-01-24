#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool wifi_connected;
    bool mqtt_connected;
    char ip[16];
    uint32_t radio_rx;
    uint32_t radio_err;
    uint32_t uptime_sec;
} system_state_t;

extern system_state_t g_state;
void system_state_init(void);