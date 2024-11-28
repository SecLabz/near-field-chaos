#include "pio_uart.h"

PIO tx_pio;
uint tx_sm;
uint tx_offset;

PIO rx_pio;
uint rx_sm;
uint rx_offset;

void pio_uart_init(uint8_t tx_pin, uint8_t rx_pin, uint baud_rate)
{
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_tx_program, &tx_pio, &tx_sm, &tx_offset, tx_pin, 1, true);
    hard_assert(success);

    success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &rx_pio, &rx_sm, &rx_offset, rx_pin, 1, true);
    hard_assert(success);

    uart_tx_program_init(tx_pio, tx_sm, tx_offset, tx_pin, baud_rate);
    uart_rx_program_init(rx_pio, rx_sm, rx_offset, rx_pin, baud_rate);
}

void pio_uart_deinit()
{
    pio_remove_program_and_unclaim_sm(&uart_tx_program, tx_pio, tx_sm, tx_offset);
    pio_remove_program_and_unclaim_sm(&uart_rx_program, rx_pio, rx_sm, rx_offset);
}

void pio_uart_putc(char c)
{
    uart_tx_program_putc(tx_pio, tx_sm, c);
}

char pio_uart_getc()
{
    return uart_rx_program_getc(rx_pio, rx_sm);
}

void pio_uart_uart_write_blocking(const uint8_t *src, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        pio_uart_putc(src[i]);
    }
}

bool pio_uart_is_readable()
{
    return !pio_sm_is_rx_fifo_empty(rx_pio, rx_sm);
}
