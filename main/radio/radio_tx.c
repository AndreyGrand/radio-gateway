#include "radio_tx.h"
#include "nrf24.h"
#include "rmessage.h"
#include "crc32.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <string.h>

#define RETRY_MAX        5
#define ACK_TIMEOUT_MS  30

static QueueHandle_t tx_queue;
static uint32_t tx_seq = 1;

/* обновляется из RX задачи */
volatile uint32_t g_last_ack_num = 0;

typedef struct {
    rmessage_t msg;
} tx_item_t;

void radio_tx_init(void)
{
    tx_queue = xQueueCreate(8, sizeof(tx_item_t));
}

bool radio_send(uint8_t dst,
                uint8_t type,
                const uint8_t *data,
                uint8_t len)
{
    if (!tx_queue || len > RMESSAGE_MAX_DATA)
        return false;

    tx_item_t item = {0};

    item.msg.hdr.dst  = dst;
    item.msg.hdr.num  = tx_seq++;
    item.msg.hdr.type = type;
    item.msg.hdr.len  = len;

    memcpy(item.msg.data, data, len);

    item.msg.crc = crc32(
        (uint8_t *)&item.msg,
        sizeof(rmsg_header_t) + len
    );

    return xQueueSend(tx_queue, &item, 0);
}

static bool wait_for_ack(uint32_t num)
{
    TickType_t start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) <
           pdMS_TO_TICKS(ACK_TIMEOUT_MS)) {

        if (g_last_ack_num == num)
            return true;

        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return false;
}

void radio_tx_task(void *arg)
{
    tx_item_t item;

    while (1) {
        if (xQueueReceive(tx_queue, &item, portMAX_DELAY)) {

            for (int attempt = 0;
                 attempt < RETRY_MAX;
                 attempt++) {

                nrf24_send(
                    (uint8_t *)&item.msg,
                    sizeof(rmsg_header_t) +
                    item.msg.hdr.len + 4
                );

                if (wait_for_ack(item.msg.hdr.num))
                    break;
            }
        }
    }
}
