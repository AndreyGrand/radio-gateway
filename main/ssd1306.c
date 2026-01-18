#include "ssd1306.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

#define I2C_PORT I2C_NUM_0
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDR 0x3C

static uint8_t buffer[128 * 64 / 8];

static void i2c_write(uint8_t control, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, control, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, 100 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

static void ssd1306_cmd(uint8_t cmd) { i2c_write(0x00, cmd); }
static void ssd1306_data(uint8_t data) { i2c_write(0x40, data); }

void ssd1306_init(void)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_PORT, &cfg);
    i2c_driver_install(I2C_PORT, cfg.mode, 0, 0, 0);

    ssd1306_cmd(0xAE); ssd1306_cmd(0x20); ssd1306_cmd(0x00);
    ssd1306_cmd(0xB0); ssd1306_cmd(0xC8); ssd1306_cmd(0x00);
    ssd1306_cmd(0x10); ssd1306_cmd(0x40); ssd1306_cmd(0x81);
    ssd1306_cmd(0x7F); ssd1306_cmd(0xA1); ssd1306_cmd(0xA6);
    ssd1306_cmd(0xA8); ssd1306_cmd(0x3F); ssd1306_cmd(0xA4);
    ssd1306_cmd(0xD3); ssd1306_cmd(0x00); ssd1306_cmd(0xD5);
    ssd1306_cmd(0x80); ssd1306_cmd(0xD9); ssd1306_cmd(0xF1);
    ssd1306_cmd(0xDA); ssd1306_cmd(0x12); ssd1306_cmd(0xDB);
    ssd1306_cmd(0x40); ssd1306_cmd(0x8D); ssd1306_cmd(0x14);
    ssd1306_cmd(0xAF);

    ssd1306_clear();
    ssd1306_update();
}

void ssd1306_clear(void) { memset(buffer, 0, sizeof(buffer)); }

void ssd1306_update(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_cmd(0xB0 + page);
        ssd1306_cmd(0x00); ssd1306_cmd(0x10);
        for (uint8_t col = 0; col < 128; col++) {
            ssd1306_data(buffer[page * 128 + col]);
        }
    }
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str)
{
    /* минимальная реализация: только для проверки */
    while (*str) {
        uint8_t c = *str++;
        if (x >= 128) { x = 0; y += 8; }
        buffer[y / 8 * 128 + x] = c; // очень примитивно, заменяется на шрифт
        x += 6;
    }
}
