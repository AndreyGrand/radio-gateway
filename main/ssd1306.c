#include "ssd1306.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

static const char *TAG = "ssd1306";

#define I2C_PORT I2C_NUM_0
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDR 0x3C

static uint8_t buffer[128 * 64 / 8];
static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

static void i2c_write(uint8_t control, uint8_t data)
{
    uint8_t write_buffer[2] = {control, data};
    i2c_master_transmit(dev_handle, write_buffer, sizeof(write_buffer), -1);
}

static void ssd1306_cmd(uint8_t cmd) { i2c_write(0x00, cmd); }
static void ssd1306_data(uint8_t data) { i2c_write(0x40, data); }

void ssd1306_init(void)
{
    // Configure I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_PORT,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

    // Configure I2C device
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

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
