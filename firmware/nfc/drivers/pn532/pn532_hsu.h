#pragma once
#include <hardware/uart.h>
#include <pico/stdlib.h>
#include <string.h>
#include <tusb.h>
#include "pico/stdlib.h"
#include "pn532.h"

#define PN532_HSU_BUFFER_SIZE 64

#define PN532_HSU_UART uart1
#define PN532_HSU_BAUD_RATE 115200
#define PN532_HSU_DATA_BITS 8
#define PN532_HSU_STOP_BITS 1
#define PN532_HSU_PARITY UART_PARITY_NONE
#define PN532_HSU_TX_PIN 4
#define PN532_HSU_RX_PIN 5

void pn532_hsu_uart_init();
void pn532_hsu_wakeup();
int8_t pn532_hsu_receive(uint8_t *buf, int len, uint16_t timeout);
int8_t pn532_hsu_read_ack();
int8_t pn532_hsu_send_command(const uint8_t *data, uint8_t len);
int16_t pn532_hsu_read_response(uint8_t buf[], uint8_t len, uint16_t timeout);
int16_t pn532_hsu_command(uint8_t *cmd, uint8_t cmd_len, uint8_t *out, uint8_t out_len, uint16_t timeout);
int8_t pn532_hsu_get_firmware(uint8_t *ic_type);
int8_t pn532_hsu_test_communication();
int pn532_hsu_usb_bridge_start();