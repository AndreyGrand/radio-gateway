#pragma once
#include <stdint.h>
#include <stdbool.h>

void nrf24_init(void);
void nrf24_set_rx_mode(void);
void nrf24_set_tx_mode(void);

bool nrf24_send(const uint8_t *data, uint8_t len);
bool nrf24_data_available(void);
uint8_t nrf24_read(uint8_t *buf, uint8_t maxlen);
bool nrf24_irq_pending(void);