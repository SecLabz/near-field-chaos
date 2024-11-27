#pragma once

#ifndef PN532_SPI_H
#define PN532_SPI_H

/*
    https://github.com/Austin-TheTrueShinobi/LivingGameBoardArchive
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pn532.h"
#include "../../../utils/utils.h"
#include "../../../spi/pio_spi.h"

#define STATUS_READ 2
#define DATA_WRITE 1
#define DATA_READ 3

typedef struct {
    spi_inst_t *spi_port;
    uint32_t sck;
    uint32_t miso;
    uint32_t mosi;
    uint32_t cs;
    uint32_t irq;
    uint32_t baudrate;
} pn532_spi_parameters;

int spi_read_blocking_lsb(pio_spi_inst_t *spi, uint8_t repeated_tx_data, uint8_t *dst, size_t len);
int spi_write_blocking_lsb(pio_spi_inst_t *spi, uint8_t *src, size_t len);
void pn532_spi_send_frame(uint8_t *data, uint8_t len);
int16_t pn532_spi_send_command(uint8_t *data, uint8_t len);
int16_t pn532_spi_read_command_response(uint8_t *buf, uint8_t len, uint16_t timeout);
int16_t pn532_spi_command(uint8_t *cmd, uint8_t cmd_len, uint8_t *out, uint8_t out_len, uint16_t timeout);
int pn532_spi_init(void *parameters);
void pn532_wakeup();
int pn532_wait_irq(uint16_t timeout);


#endif