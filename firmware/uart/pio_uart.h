#pragma once
#include "uart_tx.pio.h"
#include "uart_rx.pio.h"

void pio_uart_init(uint8_t tx_pin, uint8_t rx_pin, uint baud_rate);
void pio_uart_deinit();
void pio_uart_putc(char c);
char pio_uart_getc();
void pio_uart_uart_write_blocking(const uint8_t *src, size_t len);
bool pio_uart_is_readable();