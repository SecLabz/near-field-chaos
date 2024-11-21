#include "nfc.h"

typedef struct
{
    int (*init)(void *parameters);
    void (*reset)(void);
    void (*dispose)(void);
    void (*rf_tearoff_reset)(void);
    void (*rf_enable)(bool enable);
    int (*st25tb_init)(void);
    int (*st25tb_transceive_bytes)(uint8_t *tx_buffer, size_t tx_length, uint8_t *rx_buffer, size_t rx_length, uint8_t timeout_ms);
} nfc_driver;

typedef struct
{
    const nfc_driver *driver;
    void *parameters;
    void *context;
} nfc_device;

#if CHIP == CHIP_TRF7970A

trf7970a_spi_parameters trf7970a_spi_params = {
    .spi_port = TRF7970A_SPI_PORT,
    .sck = TRF7970A_SPI_SCK_PIN,
    .miso = TRF7970A_SPI_MISO_PIN,
    .mosi = TRF7970A_SPI_MOSI_PIN,
    .cs = TRF7970A_SPI_CS_PIN,
    .irq = TRF7970A_SPI_IRQ_PIN,
    .en = TRF7970A_EN_PIN,
    .io0 = TRF7970A_IO0_PIN,
    .io1 = TRF7970A_IO1_PIN,
    .io2 = TRF7970A_IO2_PIN,
    .baudrate = TRF7970A_SPI_BAUDRATE,
};

const nfc_driver trf7970a_spi_driver = {
    .init = trf7970a_init,
    .reset = trf7970a_reset,
    .rf_tearoff_reset = trf7970a_rf_tearoff_reset,
    .st25tb_init = trf7970a_st25tb_init,
    .rf_enable = trf7970a_rf_enable,
    .st25tb_transceive_bytes = trf7970a_st25tb_transceive_bytes,
};

nfc_device device = {
    .driver = &trf7970a_spi_driver,
    .parameters = (void *)&trf7970a_spi_params,
};

#elif CHIP == CHIP_PN532

pn532_spi_parameters pn532_spi_params = {
    .spi_port = PN532_SPI_PORT,
    .sck = PN532_SPI_SCK_PIN,
    .miso = PN532_SPI_MISO_PIN,
    .mosi = PN532_SPI_MOSI_PIN,
    .cs = PN532_SPI_CS_PIN,
    .irq = PN532_SPI_IRQ_PIN,
    .baudrate = PN532_SPI_BAUDRATE,
};

nfc_driver pn532_spi_driver = {
    .init = pn532_spi_init,
    .reset = pn532_reset,
    .rf_tearoff_reset = pn532_rf_tearoff_reset,
    .rf_enable = pn532_rf_enable,
    .st25tb_init = pn532_st25tb_init,
    .st25tb_transceive_bytes = pn532_st25tb_transceive_bytes,
};

nfc_device device = {
    .driver = &pn532_spi_driver,
    .parameters = (void *)&pn532_spi_params,
};

#endif

static nfc_device *device_ptr = NULL;

int nfc_init()
{
    device_ptr = &device;
    if (device_ptr->driver->init(device_ptr->parameters) == NFC_ERROR)
    {
        return NFC_ERROR;
    }

    return NFC_SUCCESS;
}

void nfc_deinit()
{
    if (device_ptr == NULL)
    {
        return;
    }

    device_ptr->driver->dispose();
}

void nfc_reset()
{
    if (device_ptr == NULL)
    {
        return;
    }

    device_ptr->driver->reset();
}

void nfc_rf_tearoff_reset()
{
    if (device_ptr == NULL)
    {
        return;
    }

    device_ptr->driver->rf_tearoff_reset();
}

void nfc_rf_enable(bool enable)
{
    if (device_ptr == NULL)
    {
        return;
    }

    device_ptr->driver->rf_enable(enable);
}

int nfc_st25tb_init()
{
    if (device_ptr == NULL)
    {
        return NFC_ERROR;
    }

    return device_ptr->driver->st25tb_init();
}

int nfc_st25tb_transceive_bytes(uint8_t *tx_buffer, size_t tx_length, uint8_t *rx_buffer, size_t rx_length, uint8_t timeout_ms)
{
    if (device_ptr == NULL)
    {
        return NFC_ERROR;
    }

    return device_ptr->driver->st25tb_transceive_bytes(tx_buffer, tx_length, rx_buffer, rx_length, timeout_ms);
}