#include "st25tb_cmd.h"

const uint8_t initiate_cmd[] = {ST25TB_CMD_INITIATE_0, ST25TB_CMD_INITIATE_1};
int8_t st25tb_cmd_initiate(uint8_t *chip_id)
{
    int8_t result;

    result = nfc_st25tb_transceive_bytes((uint8_t *)&initiate_cmd, sizeof(initiate_cmd), chip_id, 1, ST25TB_TRANSCEIVE_TIMEOUT_MS);
    return result == 1 ? 0 : -1;
}

int8_t st25tb_cmd_select(const uint8_t chip_id)
{
    int8_t result;
    uint8_t response;

    const uint8_t select_cmd[] = {ST25TB_CMD_SELECT, chip_id};
    result = nfc_st25tb_transceive_bytes((uint8_t *)&select_cmd, sizeof(select_cmd), &response, 1, ST25TB_TRANSCEIVE_TIMEOUT_MS);

    if (result != 1 || response != chip_id)
    {
        return -1;
    }
    return 0;
}

const uint8_t cmd_get_uid[] = {ST25TB_CMD_GET_UID};
int8_t st25tb_cmd_get_uid(uint64_t *uid)
{
    int8_t result;
    uint8_t response;

    result = nfc_st25tb_transceive_bytes((uint8_t *)&cmd_get_uid, sizeof(cmd_get_uid), (uint8_t *)uid, sizeof(*uid), ST25TB_TRANSCEIVE_TIMEOUT_MS);
    return result == 8 ? 0 : -1;
}

int8_t st25tb_cmd_read_block(const uint8_t address, uint32_t *block)
{
    int result;
    const uint8_t cmd_read_block[] = {ST25TB_CMD_READ_BLOCK, address};

    result = nfc_st25tb_transceive_bytes((uint8_t *)&cmd_read_block, sizeof(cmd_read_block), (uint8_t *)block, sizeof(*block), ST25TB_TRANSCEIVE_TIMEOUT_MS);
    return result == sizeof(block) ? 0 : -1;
}

int8_t st25tb_cmd_write_block(const uint8_t address, const uint32_t block, uint8_t timeout_ms)
{
    int result;
    const uint8_t cmd_write_block[6] = {ST25TB_CMD_WRITE_BLOCK, address};

    *(uint32_t *)(cmd_write_block + 2) = block;

    nfc_st25tb_transceive_bytes((uint8_t *)&cmd_write_block, 6, NULL, 0, timeout_ms);

    return 0;
}

const uint8_t cmd_completion[] = {ST25TB_CMD_COMPLETION};
int8_t st25tb_cmd_completion()
{
    nfc_st25tb_transceive_bytes((uint8_t *)&cmd_completion, 1, NULL, 0, ST25TB_TRANSCEIVE_TIMEOUT_MS);

    return 0;
}

const uint8_t cmd_reset_to_inventory[] = {ST25TB_CMD_RESET_TO_INVENTORY};
int8_t st25tb_cmd_reset_to_inventory()
{
    nfc_st25tb_transceive_bytes((uint8_t *)&cmd_reset_to_inventory, 1, NULL, 0, ST25TB_TRANSCEIVE_TIMEOUT_MS);

    return 0;
}