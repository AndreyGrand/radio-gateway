#pragma once
#include <stdint.h>

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_update(void);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str);
