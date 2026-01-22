#pragma once
#include <stdbool.h>
#include "rmessage.h"

void radio_queue_init(void);
bool radio_rx_push(const rmessage_t *msg);
bool radio_rx_pop(rmessage_t *msg);

