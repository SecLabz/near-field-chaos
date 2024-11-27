#include "pn532.h"

uint8_t pn532_buffer[255];

int16_t pn532_read_register(uint16_t reg, uint8_t *val)
{
    int16_t response;

    pn532_buffer[0] = PN532_COMMAND_READREGISTER;
    pn532_buffer[1] = (reg >> 8) & 0xFF;
    pn532_buffer[2] = reg & 0xFF;

    if (pn532_spi_command(pn532_buffer, 3, pn532_buffer, sizeof(pn532_buffer), 1000) < 0)
    {
        return -1;
    }

    *val = pn532_buffer[0];

    return 0;
}

int16_t pn532_sam_configuration()
{
    pn532_buffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_buffer[1] = 0x01; // mode (Normal mode, the SAM is not used; this is the default mode)
    pn532_buffer[2] = 0x14; // timeout (not used in normal mode)
    pn532_buffer[3] = 0x01; // irq pin enabled

    if (pn532_spi_command(pn532_buffer, 4, pn532_buffer, sizeof(pn532_buffer), 1000) < 0)
    {
        return -1;
    }

    return 0;
}

int16_t pn532_write_register(uint16_t reg, uint8_t val)
{
    pn532_buffer[0] = PN532_COMMAND_WRITEREGISTER;
    pn532_buffer[1] = (reg >> 8) & 0xFF;
    pn532_buffer[2] = reg & 0xFF;
    pn532_buffer[3] = val;

    if (pn532_spi_command(pn532_buffer, 4, pn532_buffer, sizeof(pn532_buffer), 1000) < 0)
    {
        return -1;
    }

    return 0;
}

int16_t pn532_write_register_with_mask(uint16_t reg, const uint8_t mask, const uint8_t val)
{
    if (mask != 0xff)
    {
        int res = 0;
        uint8_t currentValue;
        if ((res = pn532_read_register(reg, &currentValue)) < 0)
        {
            return res;
        }
        uint8_t newValue = ((val & mask) | (currentValue & (~mask)));
        if (newValue != currentValue)
        {
            return pn532_write_register(reg, newValue);
        }
    }
    else
    {
        return pn532_write_register(reg, val);
    }

    return 0;
}

int pn532_rf_enable_skip_response(bool enable)
{
    uint8_t abtCmd[] = {RFConfiguration, RFCI_FIELD, (enable) ? 0x01 : 0x00};

    pn532_spi_send_frame(abtCmd, 3);
    sleep_us(680);

    return 0;
}

void pn532_rf_enable(bool enable)
{
    uint8_t abtCmd[] = {RFConfiguration, RFCI_FIELD, (enable) ? 0x01 : 0x00};

    pn532_spi_command(abtCmd, 3, pn532_buffer, sizeof(pn532_buffer), 1000);
}

int16_t pn532_rf_timings(uint8_t atr_res_timeout, uint8_t reply_timeout)
{
    pn532_buffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_buffer[1] = 0x02;
    pn532_buffer[2] = 0x00;            // RFU
    pn532_buffer[3] = atr_res_timeout; // timeout between ATR_REQ and ATR_RES when the PN532 is in initiator mode
    pn532_buffer[4] = reply_timeout;   // timeout for InCommunicateThru or InDataExchange

    if (pn532_spi_command(pn532_buffer, 5, pn532_buffer, sizeof(pn532_buffer), 1000) < 0)
    {
        return -1;
    }

    return 0;
}

int pn532_st25tb_init()
{
    int res = -1;

    pn532_wakeup();

    if (pn532_sam_configuration() != 0)
    {
        return res;
    }

    if (pn532_write_register(PN53X_REG_CIU_Control, 0x10) != 0)
    {
        return res;
    }

    if (pn532_write_register(PN53X_REG_CIU_TxMode, 0x83) != 0)
    {
        return res;
    }

    if (pn532_write_register(PN53X_REG_CIU_RxMode, 0x83) != 0)
    {
        return res;
    }

    if (pn532_write_register(PN53X_REG_CIU_CWGsP, 0x3F) != 0) // Conductance of the P-Driver
    {
        return res;
    }

    if (pn532_write_register(PN53X_REG_CIU_ModGsP, 0x12) != 0) // Driver P-output conductance for the time of modulation
    {
        return res;
    }
    if (pn532_write_register_with_mask(PN53X_REG_CIU_TxAuto, 0xef, 0x07) != 0) // Initial RFOn, Tx2 RFAutoEn, Tx1 RFAutoEn
    {
        return res;
    }

    return 0;
}

int pn532_st25tb_transceive_bytes(uint8_t *tx_buffer, size_t tx_length, uint8_t *rx_buffer, size_t rx_length, uint8_t timeout_ms)
{
    if (rx_length > (sizeof(pn532_buffer) + 1))
    {
        return -1;
    }
    if (rx_length == 0 && timeout_ms != 0)
    {
        pn532_rf_timings(0x00, 0x07);
    }

    pn532_buffer[0] = PN532_COMMAND_INCOMMUNICATETHRU;
    memcpy(pn532_buffer + 1, tx_buffer, tx_length);
    int16_t res = pn532_spi_send_command(pn532_buffer, tx_length + 1);
    if (res < 0)
    {
        return -1;
    }

    if (rx_length == 0 && timeout_ms == 0)
    {
        return 0;
    }

    res = pn532_spi_read_command_response(pn532_buffer, rx_length + 1, timeout_ms);
    if (res < 0)
    {
        pn532_rf_timings(0x00, 0x0A);
        return res;
    }

    if (rx_length == 0)
    {
        pn532_rf_timings(0x00, 0x0A);
        return 0;
    }

    // Check for errors
    if (pn532_buffer[0] != 0x00)
    {
        return -1;
    }

    memcpy(rx_buffer, pn532_buffer + 1, rx_length);

    return rx_length;
}

void pn532_reset()
{
    pn532_rf_enable(false);
    pn532_rf_enable(true);
}

void pn532_rf_tearoff_reset()
{
    pn532_rf_enable_skip_response(false);
    pn532_rf_enable_skip_response(true);
}

int16_t pn532_init_as_target()
{
    int16_t response;

    uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET,
        5, // MODE: PICC only, Passive only

        0x04, 0x00,       // SENS_RES
        0x12, 0x34, 0x56, // NFCID1
        0x20,             // SEL_RES

        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, // FeliCaParams
        0, 0,

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // NFCID3t

        0, // length of general bytes
        0  // length of historical bytes
    };

    if (pn532_spi_command(command, sizeof(command), pn532_buffer, sizeof(pn532_buffer), 1000) < 0)
    {
        return -1;
    }

    return 0;
}

int16_t pn532_get_data(uint8_t *buf, uint8_t len)
{
    pn532_buffer[0] = PN532_COMMAND_TGGETDATA;

    int16_t status = pn532_spi_command(pn532_buffer, 1, pn532_buffer, sizeof(pn532_buffer), 2000);
    if (status < 0)
    {
        return -1;
    }

    uint16_t length = status - 1;

    if (pn532_buffer[0] != 0)
    {
        printf("status is not ok\n");
        return -5;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        buf[i] = pn532_buffer[i + 1];
    }

    return length;
}

int pn532_set_data(const uint8_t *data, uint8_t len)
{
    if (len > (sizeof(pn532_buffer) - 1))
    {
        printf("tgSetData:buffer too small\n");
        return -1;
    }

    pn532_buffer[0] = PN532_COMMAND_TGSETDATA;
    for (int8_t i = 0; i < len; i++)
    {
        pn532_buffer[i + 1] = data[i];
    }

    int16_t res = pn532_spi_command(pn532_buffer, len + 1, pn532_buffer, sizeof(pn532_buffer), 1000);
    if (res < 0)
    {
        return -1;
    }

    if (0 != pn532_buffer[0])
    {
        return -1;
    }

    return 0;
}

int pn532_in_release(const uint8_t target)
{
    pn532_buffer[0] = PN532_COMMAND_INRELEASE;
    pn532_buffer[1] = target;

    int16_t res = pn532_spi_command(pn532_buffer, 2, pn532_buffer, sizeof(pn532_buffer), 1000);
    if (res < 0)
    {
        return -1;
    }
}

int pn532_status_as_target()
{
    pn532_buffer[0] = PN532_COMMAND_TGGETTARGETSTATUS;

    int16_t res = pn532_spi_command(pn532_buffer, 1, pn532_buffer, sizeof(pn532_buffer), 1000);
    if (res < 0)
    {
        return -1;
    }

    return pn532_buffer[0];
}