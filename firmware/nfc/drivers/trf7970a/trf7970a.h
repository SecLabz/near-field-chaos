#pragma once
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "../../nfc_status.h"

#define TRF7970A_PIN_UNDEFINED 255

#define SPI_CS_SELECT(cs) gpio_put(cs, 0);
#define SPI_CS_UNSELECT(cs) \
    gpio_put(cs, 1);        \
    busy_wait_us(1);

extern void (*trf7970a_irq_callback)(void);
extern volatile bool trf7970a_irq;

// Direct commands
#define TRF7970A_CMD_IDLE 0x00
#define TRF7970A_CMD_SOFT_INIT 0x03
#define TRF7970A_CMD_RF_COLLISION 0x04
#define TRF7970A_CMD_RF_COLLISION_RESPONSE_N 0x05
#define TRF7970A_CMD_RF_COLLISION_RESPONSE_0 0x06
#define TRF7970A_CMD_FIFO_RESET 0x0F
#define TRF7970A_CMD_TRANSMIT_NO_CRC 0x10
#define TRF7970A_CMD_TRANSMIT 0x11
#define TRF7970A_CMD_DELAY_TRANSMIT_NO_CRC 0x12
#define TRF7970A_CMD_DELAY_TRANSMIT 0x13
#define TRF7970A_CMD_EOF 0x14
#define TRF7970A_CMD_CLOSE_SLOT 0x15
#define TRF7970A_CMD_BLOCK_RX 0x16
#define TRF7970A_CMD_ENABLE_RX 0x17
#define TRF7970A_CMD_TEST_INT_RF 0x18
#define TRF7970A_CMD_TEST_EXT_RF 0x19
#define TRF7970A_CMD_RX_GAIN_ADJUST 0x1A

// Address and Command Word Bit Distribution
#define TRF7970A_CMD_CONTROL 0x80
#define TRF7970A_REG_READ 0x40
#define TRF7970A_REG_WRITE 0x00
#define TRF7970A_REG_MODE_SINGLE 0x00
#define TRF7970A_REG_MODE_CONTINUOUS 0x20
#define TRF7970A_CMD_MASK(opcode) ((opcode) & 0x1F)
#define TRF7970A_DIRECT_CMD(opcode) (TRF7970A_CMD_CONTROL | TRF7970A_CMD_MASK(opcode))

// Registers addresses
#define TRF7970A_CHIP_STATUS_CTRL 0x00
#define TRF7970A_ISO_CTRL 0x01
#define TRF7970A_ISO14443B_TX_OPTIONS 0x02
#define TRF7970A_ISO14443A_HIGH_BITRATE_OPTIONS 0x03
#define TRF7970A_TX_TIMER_SETTING_H_BYTE 0x04
#define TRF7970A_TX_TIMER_SETTING_L_BYTE 0x05
#define TRF7970A_TX_PULSE_LENGTH_CTRL 0x06
#define TRF7970A_RX_NO_RESPONSE_WAIT 0x07
#define TRF7970A_RX_WAIT_TIME 0x08
#define TRF7970A_MODULATOR_SYS_CLK_CTRL 0x09
#define TRF7970A_RX_SPECIAL_SETTINGS 0x0A
#define TRF7970A_REG_IO_CTRL 0x0B
#define TRF7970A_IRQ_STATUS 0x0C
#define TRF7970A_COLLISION_IRQ_MASK 0x0D
#define TRF7970A_COLLISION_POSITION 0x0E
#define TRF7970A_RSSI_OSC_STATUS 0x0F
#define TRF7970A_SPECIAL_FCN_REG1 0x10
#define TRF7970A_SPECIAL_FCN_REG2 0x11
#define TRF7970A_RAM1 0x12
#define TRF7970A_RAM2 0x13
#define TRF7970A_ADJUTABLE_FIFO_IRQ_LEVELS 0x14
#define TRF7970A_NFC_LOW_FIELD_LEVEL 0x16
#define TRF7970A_NFCID1 0x17
#define TRF7970A_NFC_TARGET_LEVEL 0x18
#define TRF79070A_NFC_TARGET_PROTOCOL 0x19
#define TRF7970A_TEST_REGISTER1 0x1A
#define TRF7970A_TEST_REGISTER2 0x1B
#define TRF7970A_FIFO_STATUS 0x1C
#define TRF7970A_TX_LENGTH_BYTE1 0x1D
#define TRF7970A_TX_LENGTH_BYTE2 0x1E
#define TRF7970A_FIFO_IO_REGISTER 0x1F

#define TRF7970A_IRQ_STATUS_COLLISION_ERROR 0x01
#define TRF7970A_IRQ_STATUS_COLLISION_AVOID_FINISHED 0x02
#define TRF7970A_IRQ_STATUS_RF_FIELD_CHANGE 0x04
#define TRF7970A_IRQ_STATUS_PROTOCOL_ERROR 0x10
#define TRF7970A_IRQ_STATUS_RX_COMPLETE 0x40
#define TRF7970A_IRQ_STATUS_TX_COMPLETE 0x80

// Other
#define TRF7970A_MOD_CTRL_MOD_ASK_10 0x00

#define TRF7970_FIFO_MAX_SIZE 127

typedef struct
{
    spi_inst_t *spi_port;
    uint32_t sck;
    uint32_t miso;
    uint32_t mosi;
    uint32_t cs;
    uint32_t irq;
    uint32_t en;
    uint32_t io0;
    uint32_t io1;
    uint32_t io2;
    uint32_t baudrate;
} trf7970a_spi_parameters;

int trf7970a_init(void *parameters);

void trf7970a_deinit();

void trf7970a_reset();

void trf7970a_raw_direct_command(const uint8_t command);

void trf7970a_direct_command(const uint8_t command);

void trf7970a_raw_write(const uint8_t reg,
                        const uint8_t value);

void trf7970a_write(const uint8_t reg,
                    const uint8_t value);

uint8_t trf7970a_read(const uint8_t reg);

void trf7970a_read_cont(const uint8_t reg,
                        uint8_t *buffer,
                        size_t length);

void trf7970a_write_cont(const uint8_t reg,
                         uint8_t *buffer,
                         size_t length);

void trf7970a_write_packet(uint8_t *buffer,
                           size_t length,
                           bool crc);

int trf7970a_transceive_bytes(uint8_t *tx_buffer,
                              size_t tx_length,
                              uint8_t *rx_buffer,
                              size_t rx_length,
                              bool crc,
                              uint8_t timeout_ms);

int trf7970a_st25tb_init();

int trf7970a_st25tb_transceive_bytes(uint8_t *tx_buffer,
                                     size_t tx_length,
                                     uint8_t *rx_buffer,
                                     size_t rx_length,
                                     uint8_t timeout_ms);

void st25tb_deinit();

void trf7970a_rf_enable(bool enable);
void trf7970a_rf_tearoff_reset();