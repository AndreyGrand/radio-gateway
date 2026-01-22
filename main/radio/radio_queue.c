#include "radio_queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static QueueHandle_t rx_queue;

void radio_queue_init(void)
{
    rx_queue = xQueueCreate(8, sizeof(rmessage_t));
}

bool radio_rx_push(const rmessage_t *msg)
{
    if (!rx_queue) return false;
    return xQueueSend(rx_queue, msg, 0);
}

bool radio_rx_pop(rmessage_t *msg)
{
    if (!rx_queue) return false;
    return xQueueReceive(rx_queue, msg, 0);
}
