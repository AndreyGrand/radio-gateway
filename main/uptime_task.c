#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_state.h"

void uptime_task(void *arg)
{
    while (1) {
        g_state.uptime_sec++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
