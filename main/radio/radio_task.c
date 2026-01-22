#include "nrf24.h"
#include "rmessage.h"
#include "crc32.h"
#include "radio_queue.h"
#include "radio_tx.h"
#include "system_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern volatile uint32_t g_last_ack_num;

void radio_task(void *arg)
{
    uint8_t buf[32];

    nrf24_init();
    nrf24_set_rx_mode();

    while (1) {
        if (nrf24_irq_pending()) {

            nrf24_read(buf, sizeof(buf));

            /* ACK packet */
            if (buf[0] == 0xFF) {
                rack_t *ack = (rack_t *)buf;
                g_last_ack_num = ack->num;
                continue;
            }

            rmessage_t *msg = (rmessage_t *)buf;

            uint32_t crc = crc32(
                buf,
                sizeof(rmsg_header_t) + msg->hdr.len
            );

            if (crc != msg->crc) {
                g_state.radio_err++;
                continue;
            }

            g_state.radio_rx++;
            radio_rx_push(msg);

            rack_t ack = {
                .dst = msg->hdr.dst,
                .num = msg->hdr.num
            };

            ack.crc = crc32(
                (uint8_t *)&ack,
                sizeof(ack) - 4
            );

            nrf24_send((uint8_t *)&ack, sizeof(ack));
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
