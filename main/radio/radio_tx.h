#pragma once
#include <stdint.h>
#include <stdbool.h>

bool radio_send(uint8_t dst,
                uint8_t type,
                const uint8_t *data,
                uint8_t len);

void radio_tx_init(void);
void radio_tx_task(void *arg);
