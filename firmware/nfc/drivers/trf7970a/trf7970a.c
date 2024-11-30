#include "trf7970a.h"

void (*trf7970a_irq_callback)(void) = NULL;
volatile bool trf7970a_irq = false;

volatile bool timer_fired = false;
uint8_t cs_pin;
uint8_t irq_pin;
spi_inst_t *spi;

void trf7970a_irq_isr_callback(uint gpio, uint32_t events)
{
    trf7970a_irq = true;
    if (trf7970a_irq_callback != NULL)
    {
        trf7970a_irq_callback();
    }
}

int trf7970a_init(void *parameters)
{
    trf7970a_spi_parameters *params = (trf7970a_spi_parameters *)parameters;
    spi = params->spi_port;
    cs_pin = params->cs;
    irq_pin = params->irq;

    // initialize CS pin high
    gpio_init(params->cs);
    gpio_set_dir(params->cs, GPIO_OUT);
    gpio_put(params->cs, 1);

    // initialize SPI port
    spi_init(params->spi_port, params->baudrate);

    // set SPI format
    spi_set_format(params->spi_port, // SPI instance
                   8,                // number of bits per transfer
                   0,                // polarity (CPOL)
                   1,                // phase (CPHA)
                   SPI_MSB_FIRST);

    // initialize SPI pins
    gpio_set_function(params->sck, GPIO_FUNC_SPI);
    gpio_set_function(params->mosi, GPIO_FUNC_SPI);
    gpio_set_function(params->miso, GPIO_FUNC_SPI);

    // enable trf7970
    gpio_init(params->en);
    gpio_set_dir(params->en, GPIO_OUT);
    gpio_put(params->en, 1);

    // setup spi pins on trf7970
    if (params->io0 != TRF7970A_PIN_UNDEFINED)
    {
        // IO0 to GND
        gpio_init(params->io0);
        gpio_set_dir(params->io0, GPIO_OUT);
        gpio_put(params->io0, 0);
    }

    if (params->io1 != TRF7970A_PIN_UNDEFINED)
    {
        // IO1 to 3.3V
        gpio_init(params->io1);
        gpio_set_dir(params->io1, GPIO_OUT);
        gpio_put(params->io1, 1);
    }

    if (params->io2 != TRF7970A_PIN_UNDEFINED)
    {
        // IO2 to 3.3V
        gpio_init(params->io2);
        gpio_set_dir(params->io2, GPIO_OUT);
        gpio_put(params->io2, 1);
    }

    sleep_ms(10);

    gpio_set_irq_enabled_with_callback(irq_pin, GPIO_IRQ_EDGE_RISE, true, &trf7970a_irq_isr_callback);

    return NFC_SUCCESS;
}

void trf7970a_deinit()
{
    trf7970a_direct_command(TRF7970A_CMD_SOFT_INIT);
    irq_set_enabled(irq_pin, false);
    gpio_set_irq_enabled_with_callback(irq_pin, GPIO_IRQ_EDGE_RISE, false, &trf7970a_irq_isr_callback);
    timer_fired = false;
    trf7970a_irq_callback = NULL;
    trf7970a_irq = false;
}

void trf7970a_reset()
{
    trf7970a_direct_command(TRF7970A_CMD_SOFT_INIT);
}

void trf7970a_raw_direct_command(const uint8_t command)
{
    uint8_t cmd = TRF7970A_DIRECT_CMD(command);

    spi_write_blocking(spi, &cmd, 1);
}

void trf7970a_direct_command(const uint8_t command)
{
    uint8_t cmd = TRF7970A_DIRECT_CMD(command);

    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, &cmd, 1);
    SPI_CS_UNSELECT(cs_pin);
}

void trf7970a_raw_write(const uint8_t reg,
                        const uint8_t value)
{
    uint8_t address = TRF7970A_CMD_MASK(reg);
    const uint8_t buffer[2] = {address, value};

    spi_write_blocking(spi, (uint8_t *)&buffer, 2);
}

void trf7970a_write(const uint8_t reg,
                    const uint8_t value)
{
    uint8_t address = TRF7970A_CMD_MASK(reg);
    const uint8_t buffer[2] = {address, value};
    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, (uint8_t *)&buffer, 2);
    SPI_CS_UNSELECT(cs_pin);
}

int8_t trf7970a_raw_read(const uint8_t reg)
{
    int8_t result;

    uint8_t address = TRF7970A_CMD_MASK(reg) | TRF7970A_REG_READ;
    spi_write_blocking(spi, &address, 1);
    spi_read_blocking(spi, 0, &result, 1);

    return result;
}

void trf7970a_raw_read_cont(const uint8_t reg,
                            uint8_t *buffer,
                            size_t length)
{
    uint8_t address = TRF7970A_CMD_MASK(reg) | TRF7970A_REG_READ | TRF7970A_REG_MODE_CONTINUOUS;

    spi_write_blocking(spi, &address, 1);
    spi_read_blocking(spi, 0, buffer, length);
}

int8_t trf7970a_read(const uint8_t reg)
{
    int8_t result;

    uint8_t address = TRF7970A_CMD_MASK(reg) | TRF7970A_REG_READ;

    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, &address, 1);
    spi_read_blocking(spi, 0, &result, 1);
    SPI_CS_UNSELECT(cs_pin);

    return result;
}

void trf7970a_read_cont(const uint8_t reg,
                        uint8_t *buffer,
                        size_t length)
{
    uint8_t address = TRF7970A_CMD_MASK(reg) | TRF7970A_REG_READ | TRF7970A_REG_MODE_CONTINUOUS;
    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, &address, 1);
    spi_read_blocking(spi, 0, buffer, length);
    SPI_CS_UNSELECT(cs_pin);
}

void trf7970a_write_cont(const uint8_t reg,
                         uint8_t *buffer,
                         size_t length)
{
    uint8_t address = TRF7970A_CMD_MASK(reg) | TRF7970A_REG_WRITE | TRF7970A_REG_MODE_CONTINUOUS;

    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, &address, 1);
    spi_write_blocking(spi, buffer, length);
    SPI_CS_UNSELECT(cs_pin);
}

void trf7970a_write_packet(uint8_t *buffer,
                           size_t length,
                           bool crc)
{
    uint8_t fifo_reset_cmd, tansmit_cmd, tx_length_byte1_addr, low_length, high_length;

    uint16_t ui16length = length;
    low_length = (ui16length & 0x0f) << 4;
    high_length = (ui16length & 0x0FF0) >> 4;

    fifo_reset_cmd = TRF7970A_DIRECT_CMD(TRF7970A_CMD_FIFO_RESET);
    if (crc)
    {
        tansmit_cmd = TRF7970A_DIRECT_CMD(TRF7970A_CMD_TRANSMIT);
    }
    else
    {
        tansmit_cmd = TRF7970A_DIRECT_CMD(TRF7970A_CMD_TRANSMIT_NO_CRC);
    }

    tx_length_byte1_addr = TRF7970A_CMD_MASK(TRF7970A_TX_LENGTH_BYTE1) | TRF7970A_REG_WRITE | TRF7970A_REG_MODE_CONTINUOUS;

    SPI_CS_SELECT(cs_pin);
    spi_write_blocking(spi, &fifo_reset_cmd, 1);
    spi_write_blocking(spi, &tansmit_cmd, 1);
    spi_write_blocking(spi, &tx_length_byte1_addr, 1);
    spi_write_blocking(spi, &high_length, 1);
    spi_write_blocking(spi, &low_length, 1);
    spi_write_blocking(spi, buffer, length);
    SPI_CS_UNSELECT(cs_pin);

    // wait for tx finished irq ?
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    timer_fired = true;
    return 0;
}

int trf7970a_transceive_bytes(uint8_t *tx_buffer,
                              size_t tx_length,
                              uint8_t *rx_buffer,
                              size_t rx_length,
                              bool crc,
                              uint8_t timeout_ms)
{
    alarm_id_t alarm_id;

    if (rx_length == 0)
    {
        trf7970a_write_packet(tx_buffer, tx_length, crc);
        return 0;
    }

    timer_fired = false;
    alarm_id = add_alarm_in_ms(timeout_ms, alarm_callback, NULL, false);

    trf7970a_write_packet(tx_buffer, tx_length, crc);
    bool rx_finished = false;

    while (!timer_fired)
    {
        if (trf7970a_irq)
        {
            trf7970a_irq = false;

            // Read IRQ + Dummy read
            // uint8_t read_buffer[4];
            // trf7970a_read_cont(TRF7970A_IRQ_STATUS, read_buffer, 4);
            // uint8_t irq_status = read_buffer[0];
            uint8_t irq_status = trf7970a_read(TRF7970A_IRQ_STATUS);

            if (irq_status & 0x80) // Tx finished
            {
                // Sleep to fix no response from TRF every 2 read_tag
                // TODO: find how to avoid this
                sleep_us(600);
                trf7970a_direct_command(TRF7970A_CMD_FIFO_RESET);
            }
            else if (irq_status & 0x40)
            {
                rx_finished = true;
                if (!timer_fired)
                {
                    cancel_alarm(alarm_id);
                }
                break;
            }
        }
    }

    if (rx_finished)
    {
        uint8_t fifo_size = trf7970a_read(TRF7970A_FIFO_STATUS) & 0x7F; // clear overflow
        if (fifo_size > 0)
        {
            if (fifo_size > rx_length)
            {
                fifo_size = rx_length;
            }

            trf7970a_read_cont(TRF7970A_FIFO_IO_REGISTER, rx_buffer, fifo_size);
            return fifo_size;
        }
    }

    return 0;
}

int trf7970a_st25tb_init()
{
    gpio_put(cs_pin, 0);

    trf7970a_raw_direct_command(TRF7970A_CMD_SOFT_INIT);
    trf7970a_raw_direct_command(TRF7970A_CMD_IDLE);

    trf7970a_raw_direct_command(TRF7970A_CMD_FIFO_RESET);
    trf7970a_raw_write(TRF7970A_ISO_CTRL, 0x0C);
    trf7970a_raw_write(TRF7970A_MODULATOR_SYS_CLK_CTRL, 0x00);
    trf7970a_raw_write(TRF7970A_RX_SPECIAL_SETTINGS, 0x30);
    trf7970a_raw_write(TRF7970A_ISO14443A_HIGH_BITRATE_OPTIONS, 0x00);

    trf7970a_raw_write(TRF7970A_REG_IO_CTRL, 0x00);
    trf7970a_raw_write(TRF7970A_ADJUTABLE_FIFO_IRQ_LEVELS, 0x0F);
    trf7970a_raw_write(TRF7970A_NFC_LOW_FIELD_LEVEL, 0x03);
    trf7970a_raw_write(TRF7970A_NFC_TARGET_LEVEL, 0x07);

    // Turn RF field On
    trf7970a_raw_write(TRF7970A_CHIP_STATUS_CTRL, 0x20);

    gpio_put(cs_pin, 1);
    sleep_ms(5);

    return NFC_SUCCESS;
}

int trf7970a_st25tb_transceive_bytes(uint8_t *tx_buffer,
                                     size_t tx_length,
                                     uint8_t *rx_buffer,
                                     size_t rx_length,
                                     uint8_t timeout_ms)
{
    return trf7970a_transceive_bytes(tx_buffer, tx_length, rx_buffer, rx_length, true, timeout_ms);
}

void trf7970a_rf_enable(bool enable)
{
    trf7970a_write(TRF7970A_CHIP_STATUS_CTRL, enable ? 0x20 : 0x00 );
    sleep_us(580); // min
}

void trf7970a_rf_tearoff_reset()
{
    trf7970a_rf_enable(false);
    trf7970a_rf_enable(true);
}

void st25tb_deinit()
{
    trf7970a_deinit();
}