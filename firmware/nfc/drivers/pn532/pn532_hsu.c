#include "pn532_hsu.h"

uint8_t hsu_command_num = 0;

void pn532_hsu_uart_init()
{
    pio_uart_init(PN532_HSU_TX_PIN, PN532_HSU_RX_PIN, PN532_HSU_BAUD_RATE);
}

void pn532_hsu_uart_deinit()
{
    pio_uart_deinit();
}

void pn532_hsu_wakeup()
{
    pio_uart_putc(0x55);
    pio_uart_putc(0x55);
    pio_uart_putc(0);
    pio_uart_putc(0);
    pio_uart_putc(0);

    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0x00);
    pio_uart_putc(0xFF);
    pio_uart_putc(0x05);
    pio_uart_putc(0xFB);
    pio_uart_putc(0xD4);
    pio_uart_putc(0x14);
    pio_uart_putc(0x01);
    pio_uart_putc(0x14);
    pio_uart_putc(0x01);
    pio_uart_putc(0x02);
    pio_uart_putc(0x00);

    sleep_ms(3);

    // clear serial buffer
    while (pio_uart_is_readable())
    {
        pio_uart_getc();
    }

    sleep_ms(3);
}

int8_t pn532_hsu_receive(uint8_t *buf, int len, uint16_t timeout)
{
    int read_bytes = 0;
    int ret;
    unsigned long start_millis;

    while (read_bytes < len)
    {
        ret = -1;
        start_millis = to_ms_since_boot(get_absolute_time());
        do
        {
            if (pio_uart_is_readable())
            {
                ret = pio_uart_getc();
                if (ret >= 0)
                {
                    break;
                }
            }

        } while ((timeout == 0) || ((to_ms_since_boot(get_absolute_time()) - start_millis) < timeout));

        if (ret < 0)
        {
            if (read_bytes)
            {
                return read_bytes;
            }
            else
            {
                return PN532_TIMEOUT;
            }
        }
        buf[read_bytes] = (uint8_t)ret;
        read_bytes++;
    }
    return read_bytes;
}

int8_t pn532_hsu_read_ack()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ack_buffer[sizeof(PN532_ACK)];

    if (pn532_hsu_receive(ack_buffer, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0)
    {
        return PN532_TIMEOUT;
    }

    if (memcmp(ack_buffer, PN532_ACK, sizeof(PN532_ACK)))
    {
        return PN532_INVALID_ACK;
    }
    return 0;
}

int8_t pn532_hsu_send_command(const uint8_t *data, uint8_t len)
{
    // clear serial buffer
    while (pio_uart_is_readable())
    {
        pio_uart_getc();
    }

    hsu_command_num = data[0];

    pio_uart_putc(PN532_PREAMBLE);
    pio_uart_putc(PN532_STARTCODE1);
    pio_uart_putc(PN532_STARTCODE2);

    // length of data field: TFI + DATA
    uint8_t length = len + 1;
    pio_uart_putc(length);

    // checksum of length
    pio_uart_putc(~length + 1);

    pio_uart_putc(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;

    for (uint8_t i = 0; i < len; i++)
    {
        sum += data[i];
        pio_uart_putc(data[i]);
    }

    uint8_t checksum = ~sum + 1;
    pio_uart_putc(checksum);
    pio_uart_putc(PN532_POSTAMBLE);

    return pn532_hsu_read_ack();
}

int16_t pn532_hsu_read_response(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];

    // frame preamble and start code
    if (pn532_hsu_receive(tmp, 3, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != tmp[0] || 0 != tmp[1] || 0xFF != tmp[2])
    {
        return PN532_INVALID_FRAME;
    }

    // receive length and check
    uint8_t length[2];
    if (pn532_hsu_receive(length, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(length[0] + length[1]))
    {
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if (length[0] > len)
    {
        return PN532_NO_SPACE;
    }

    // receive command byte
    uint8_t cmd = hsu_command_num + 1;
    if (pn532_hsu_receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (PN532_PN532TOHOST != tmp[0] || cmd != tmp[1])
    {
        return PN532_INVALID_FRAME;
    }

    if (pn532_hsu_receive(buf, length[0], timeout) != length[0])
    {
        return PN532_TIMEOUT;
    }

    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length[0]; i++)
    {
        sum += buf[i];
    }

    // checksum and postamble
    if (pn532_hsu_receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1])
    {
        return PN532_INVALID_FRAME;
    }

    return length[0];
}

int16_t pn532_hsu_command(uint8_t *cmd, uint8_t cmd_len, uint8_t *out, uint8_t out_len, uint16_t timeout)
{
    int16_t res = pn532_hsu_send_command(cmd, cmd_len);
    if (res)
    {
        return -1;
    }

    res = pn532_hsu_read_response(out, out_len, timeout);
    return res;
}

int8_t pn532_hsu_get_firmware(uint8_t *ic)
{
    uint8_t pn532_buffer[255];
    pn532_buffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (pn532_hsu_command(pn532_buffer, 1, pn532_buffer, 4, 1000) < 0)
    {
        return -1;
    }

    *ic = pn532_buffer[0];
    return 0;
}

int8_t pn532_hsu_test_communication()
{
    uint8_t ic = 0;

    pn532_hsu_uart_init();
    pn532_hsu_wakeup();

    if (pn532_hsu_get_firmware(&ic) != 0 || ic != PN532_IC)
    {
        pn532_hsu_uart_deinit();
        return -1;
    }

    pn532_hsu_uart_deinit();
    return 0;
}

/// USB bridge

uint8_t uart_buffer[PN532_HSU_BUFFER_SIZE];
uint32_t uart_position;
uint8_t usb_buffer[PN532_HSU_BUFFER_SIZE];
uint32_t usb_position;

void usb_read_bytes()
{
    uint32_t len = tud_cdc_available();

    if (len)
    {
        len = MIN(len, PN532_HSU_BUFFER_SIZE - usb_position);
        if (len)
        {
            uint32_t count;

            count = tud_cdc_read(usb_buffer, len);
            usb_position += count;
        }
    }
}

void usb_write_bytes()
{
    if (uart_position)
    {
        uint32_t count;

        count = tud_cdc_write(uart_buffer, uart_position);
        if (count < uart_position)
        {
            memcpy(uart_buffer, &uart_buffer[count],
                   uart_position - count);
        }
        uart_position -= count;

        if (count)
        {
            tud_cdc_write_flush();
        }
    }
}

void uart_read_bytes()
{
    if (pio_uart_is_readable())
    {
        while (pio_uart_is_readable() &&
               uart_position < PN532_HSU_BUFFER_SIZE)
        {
            uart_buffer[uart_position] = pio_uart_getc();
            uart_position++;
        }
    }
}

void uart_write_bytes()
{
    if (usb_position)
    {
        pio_uart_uart_write_blocking(usb_buffer, usb_position);
        usb_position = 0;
    }
}

int pn532_hsu_usb_bridge_start()
{
    uart_position = 0;
    usb_position = 0;
    pn532_hsu_uart_init();
    tusb_init();

    while (true)
    {
        tud_task();
        if (tud_cdc_connected())
        {
            usb_read_bytes();
            usb_write_bytes();
        }
        uart_read_bytes();
        uart_write_bytes();
    }

    return 0;
}