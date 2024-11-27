#include "pn532_spi.h"

pio_spi_inst_t pio_spi = {
    .pio = pio1,
    .sm = 0};
pio_spi_inst_t *spi_port = &pio_spi;
uint8_t pn532_cs_pin;
uint8_t pn532_irq_pin;

volatile bool pn532_irq = false;
volatile bool pn532_timer_fired = false;

uint8_t data_read = DATA_READ;
uint8_t data_write = DATA_WRITE;
uint8_t status_read = STATUS_READ;

uint8_t command_num = 0;

// based from https://github.com/Austin-TheTrueShinobi/LivingGameBoardArchive/blob/main/LivingBoardGameMainBoard-master/pn532/PN532_SPI.c
// TODO: Handle spi lsb with PIO

uint8_t PN532_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
uint8_t PN532_FRAME_START[] = {PN532_PREAMBLE, PN532_STARTCODE1, PN532_STARTCODE2};

int spi_write_blocking_lsb(pio_spi_inst_t *spi, uint8_t *src, size_t len)
{
    reverse_bits(src, len);
    pio_spi_write8_blocking(spi, src, len);
    reverse_bits(src, len);
}

int spi_read_blocking_lsb(pio_spi_inst_t *spi, uint8_t repeated_tx_data, uint8_t *dst, size_t len)
{
    pio_spi_read8_blocking(spi, dst, len);
    reverse_bits(dst, len);
}

void pn532_spi_irq_callback(uint gpio, uint32_t events)
{
    pn532_irq = true;
}

int64_t pn532_alarm_callback(alarm_id_t id, void *user_data)
{
    pn532_timer_fired = true;
    return 0;
}

int pn532_wait_irq(uint16_t timeout)
{
    alarm_id_t alarm_id;
    pn532_timer_fired = false;
    alarm_id = add_alarm_in_ms(timeout, pn532_alarm_callback, NULL, false);

    while (!pn532_timer_fired)
    {
        if (pn532_irq)
        {
            if (!pn532_timer_fired)
            {
                cancel_alarm(alarm_id);
            }
            pn532_irq = false;
            return 0;
        }
    }

    return -1;
}

int pn532_spi_init(void *parameters)
{
    pn532_spi_parameters *params = (pn532_spi_parameters *)parameters;
    pn532_cs_pin = params->cs;
    pn532_irq_pin = params->irq;

    // initialize CS pin high
    gpio_init(pn532_cs_pin);
    gpio_set_dir(pn532_cs_pin, GPIO_OUT);
    gpio_put(pn532_cs_pin, 1);

    gpio_set_irq_enabled_with_callback(params->irq, GPIO_IRQ_EDGE_FALL, true, &pn532_spi_irq_callback);
    
    // initialize SPI
    spi_port = &pio_spi;
    float clkdiv = 15.625f; // 4 MHz spi @ 125Mhz clk_sys
    uint cpha0_prog_offs = pio_add_program(pio_spi.pio, &spi_cpha0_program);
    pio_spi_init(pio_spi.pio, pio_spi.sm,
                 cpha0_prog_offs,
                 8, // 8 bits per SPI frame
                 clkdiv,
                 0,
                 0, // cpol 0
                 params->sck,
                 params->mosi,
                 params->miso);

    return 0;
}

void pn532_wakeup()
{
    gpio_put(pn532_cs_pin, false);
    sleep_ms(2);
    gpio_put(pn532_cs_pin, true);
}

int8_t pn532_spi_read_ack()
{
    uint8_t ack_buffer[sizeof(PN532_ACK)];
    gpio_put(pn532_cs_pin, false);
    spi_write_blocking_lsb(spi_port, &data_read, 1);
    spi_read_blocking_lsb(spi_port, 0xFF, ack_buffer, sizeof(PN532_ACK));
    gpio_put(pn532_cs_pin, true);

    return (0 == memcmp(PN532_ACK, ack_buffer, sizeof(PN532_ACK)));
}

void pn532_spi_send_frame(uint8_t *data, uint8_t len)
{
    // - Preamble (0x00)
    // - Start code  (0x00, 0xFF)
    // - Command length (x byte)
    // - Command length checksum
    // - Command bytes
    // - Checksum
    // - Postamble (0x00)

    gpio_put(pn532_cs_pin, false);

    // Declaring a write
    spi_write_blocking_lsb(spi_port, &data_write, 1);

    // Writing preamble & Start code
    spi_write_blocking_lsb(spi_port, PN532_FRAME_START, 3);

    // Length of data
    uint8_t total_len = len + 1;
    spi_write_blocking_lsb(spi_port, &total_len, 1);

    // Checksum
    total_len = ~total_len + 1;
    spi_write_blocking_lsb(spi_port, &total_len, 1);

    // Writing Rest
    total_len = PN532_HOSTTOPN532;
    spi_write_blocking_lsb(spi_port, &total_len, 1);

    // Sending data
    spi_write_blocking_lsb(spi_port, data, len);

    // Summing the data
    uint8_t sum = PN532_HOSTTOPN532;
    for (int i = 0; i < len; i++)
    {
        sum += data[i];
    }

    // Checksum
    sum = ~sum + 1;
    spi_write_blocking_lsb(spi_port, &sum, 1);
    len = PN532_POSTAMBLE;
    spi_write_blocking_lsb(spi_port, &len, 1);
    gpio_put(pn532_cs_pin, true);
}

int16_t pn532_spi_send_command(uint8_t *data, uint8_t len)
{
    pn532_irq = false;
    command_num = data[0];
    pn532_spi_send_frame(data, len);

    uint8_t timeout = PN532_ACK_WAIT_TIME;

    if (pn532_wait_irq(5) != 0)
    {
        printf("Irq timeout\n");
        return -2;
    }
    if (!pn532_spi_read_ack())
    {
        printf("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }

    return 0;
}

int16_t pn532_spi_read_command_response(uint8_t *buf, uint8_t len, uint16_t timeout)
{
    uint16_t timeout_attempts = 0;
    uint8_t cmp_buf[3], excess_buf;

    if (timeout == 0 || pn532_wait_irq(timeout) != 0)
    {
        return PN532_TIMEOUT;
    }

    gpio_put(pn532_cs_pin, false);

    int16_t result;
    do
    {
        // Declaring a read
        spi_write_blocking_lsb(spi_port, &data_read, 1);

        spi_read_blocking_lsb(spi_port, 0xFF, cmp_buf, 3);
        if (0 != memcmp(cmp_buf, PN532_FRAME_START, 3))
        {
            printf("Invalid Start\n");
            result = PN532_INVALID_FRAME;
            break;
        }

        // Both length and length checksum
        spi_read_blocking_lsb(spi_port, 0xFF, cmp_buf, 2);
        uint8_t length = cmp_buf[0];
        if ((uint8_t)(cmp_buf[0] + cmp_buf[1]) != 0)
        {
            printf("Bad Length");
            result = PN532_INVALID_FRAME;
            break;
        }

        // Getting both command and direction
        uint8_t cmd = command_num + 1;
        spi_read_blocking_lsb(spi_port, 0xFF, cmp_buf, 2);
        if (PN532_PN532TOHOST != cmp_buf[0] || cmd != cmp_buf[1])
        {
            printf("Bad command/return\n");
            result = PN532_INVALID_FRAME;
            break;
        }

        length -= 2;

        // Checking if the provided buffer is large enough
        if (length > len)
        {
            for (uint8_t i = 0; i < length; i++)
            {
                spi_read_blocking_lsb(spi_port, 0xFF, &excess_buf, 1);
            }
            printf("\nNot enough space\n");
            spi_read_blocking_lsb(spi_port, 0xFF, cmp_buf, 2);
            result = PN532_NO_SPACE;
            break;
        }

        // Retreiving the message and checksum
        uint8_t sum = PN532_PN532TOHOST + cmd;
        for (uint8_t i = 0; i < length; i++)
        {
            spi_read_blocking_lsb(spi_port, 0xFF, &(buf[i]), 1);
            sum += buf[i];
        }

        // Read Message Checksum
        spi_read_blocking_lsb(spi_port, 0xFF, &excess_buf, 1);
        if (0 != (uint8_t)(sum + excess_buf))
        {
            printf("Checksum not correct\n");
            result = PN532_INVALID_FRAME;
            break;
        }

        spi_read_blocking_lsb(spi_port, 0xFF, &excess_buf, 1);
        result = length;

    } while (0);
    gpio_put(pn532_cs_pin, true);
    return result;
}

int16_t pn532_spi_command(uint8_t *cmd, uint8_t cmd_len, uint8_t *out, uint8_t out_len, uint16_t timeout)
{
    int16_t res = pn532_spi_send_command(cmd, cmd_len);
    if (res)
    {
        return -1;
    }

    res = pn532_spi_read_command_response(out, out_len, timeout);

    return res;
}
