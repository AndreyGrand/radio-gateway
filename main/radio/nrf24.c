// • data rate: 250 kbps  (макс дальность)
// • power:    0 dBm
// • channel:  76 (2.476 GHz)
// • payload:  dynamic
// • auto-ack: OFF (мы делаем свой ACK!)
#include "nrf24.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

#define TAG "NRF24"

/* GPIO */
#define NRF24_CE   27
#define NRF24_CSN  5
#define NRF24_IRQ  26

/* SPI */
#define SPI_HOST   SPI2_HOST

/* Registers */
#define REG_CONFIG     0x00
#define REG_EN_AA      0x01
#define REG_RF_CH      0x05
#define REG_RF_SETUP   0x06
#define REG_STATUS     0x07
#define REG_RX_PW_P0   0x11
#define REG_FIFO_STATUS 0x17
#define REG_DYNPD      0x1C
#define REG_FEATURE    0x1D

/* Commands */
#define CMD_R_RX_PAYLOAD  0x61
#define CMD_W_TX_PAYLOAD  0xA0
#define CMD_FLUSH_TX     0xE1
#define CMD_FLUSH_RX     0xE2
#define CMD_W_REGISTER   0x20
#define CMD_R_REGISTER   0x00

static spi_device_handle_t spi;

/* --- helpers --- */
static inline void csn_low(void)  { gpio_set_level(NRF24_CSN, 0); }
static inline void csn_high(void) { gpio_set_level(NRF24_CSN, 1); }
static inline void ce_low(void)   { gpio_set_level(NRF24_CE, 0); }
static inline void ce_high(void)  { gpio_set_level(NRF24_CE, 1); }

static uint8_t spi_rw(uint8_t data)
{
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
        .rx_buffer = &data
    };
    spi_device_transmit(spi, &t);
    return data;
}

static void write_reg(uint8_t reg, uint8_t val)
{
    csn_low();
    spi_rw(CMD_W_REGISTER | reg);
    spi_rw(val);
    csn_high();
}

static uint8_t read_reg(uint8_t reg)
{
    csn_low();
    spi_rw(CMD_R_REGISTER | reg);
    uint8_t v = spi_rw(0xFF);
    csn_high();
    return v;
}

/* --- API --- */
void nrf24_init(void)
{
    gpio_config_t io = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << NRF24_CE) | (1ULL << NRF24_CSN)
    };
    gpio_config(&io);

    gpio_set_direction(NRF24_IRQ, GPIO_MODE_INPUT);

    ce_low();
    csn_high();

    spi_bus_config_t buscfg = {
        .mosi_io_num = 23,
        .miso_io_num = 19,
        .sclk_io_num = 18,
    };

    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1
    };

    spi_bus_add_device(SPI_HOST, &devcfg, &spi);

    write_reg(REG_CONFIG, 0x0C);   // PWR_UP, CRC enabled
    write_reg(REG_EN_AA, 0x00);    // auto-ack OFF
    write_reg(REG_RF_CH, 76);
    write_reg(REG_RF_SETUP, 0x26); // 250kbps, 0dBm
    write_reg(REG_FEATURE, 0x04);  // dynamic payload
    write_reg(REG_DYNPD, 0x01);

    csn_low(); spi_rw(CMD_FLUSH_RX); csn_high();
    csn_low(); spi_rw(CMD_FLUSH_TX); csn_high();

    ESP_LOGI(TAG, "NRF24 initialized");
}

void nrf24_set_rx_mode(void)
{
    ce_low();
    write_reg(REG_CONFIG, 0x0F); // PRX
    ce_high();
}

bool nrf24_send(const uint8_t *data, uint8_t len)
{
    ce_low();
    csn_low();
    spi_rw(CMD_W_TX_PAYLOAD);
    for (int i = 0; i < len; i++)
        spi_rw(data[i]);
    csn_high();
    ce_high();
    vTaskDelay(pdMS_TO_TICKS(2));
    ce_low();
    return true;
}

bool nrf24_irq_pending(void)
{
    return gpio_get_level(NRF24_IRQ) == 0;
}

uint8_t nrf24_read(uint8_t *buf, uint8_t maxlen)
{
    csn_low();
    spi_rw(CMD_R_RX_PAYLOAD);
    for (int i = 0; i < maxlen; i++)
        buf[i] = spi_rw(0xFF);
    csn_high();

    write_reg(REG_STATUS, 0x40); // clear RX_DR
    return maxlen;
}
