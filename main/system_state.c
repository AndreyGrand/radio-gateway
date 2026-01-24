#include "system_state.h"
#include <string.h>

/* реальное хранилище */
system_state_t g_state;

void system_state_init(void)
{
    memset(&g_state, 0, sizeof(g_state));
}
