#include "radio/nrf24.h"
#include "radio/rmessage.h"
#include "radio/crc32.h"
#include "system_state.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include <stdio.h>

void radio_task(void *arg)
{
    uint8_t buf[32];

    nrf24_init();
    nrf24_set_rx_mode();

    while (1) {
        if (nrf24_irq_pending()) {

            nrf24_read(buf, sizeof(buf));
            rmessage_t *msg = (rmessage_t*)buf;

            uint32_t crc = crc32(
                buf,
                sizeof(rmsg_header_t) + msg->hdr.len
            );

            if (crc != msg->crc) {
                g_state.radio_err++;
                continue;
            }

            g_state.radio_rx++;

            rack_t ack = {
                .dst = msg->hdr.dst,
                .num = msg->hdr.num,
            };

            ack.crc = crc32((uint8_t*)&ack, sizeof(ack)-4);
            nrf24_send((uint8_t*)&ack, sizeof(ack));
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
