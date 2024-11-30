#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "st25tb_common.h"
#include "../utils/utils.h"
#include "../nfc/nfc.h"

#define ST25TB_TRANSCEIVE_TIMEOUT_MS 20

#define ST25TB_CMD_INITIATE_0 0x06
#define ST25TB_CMD_INITIATE_1 0x00
#define ST25TB_CMD_PCALL16_0 0x06
#define ST25TB_CMD_PCALL16_1 0x04
#define ST25TB_CMD_SLOT_MARKER 0x06
#define ST25TB_CMD_READ_BLOCK 0x08
#define ST25TB_CMD_WRITE_BLOCK 0x09
#define ST25TB_CMD_GET_UID 0x0B
#define ST25TB_CMD_RESET_TO_INVENTORY 0x0C
#define ST25TB_CMD_SELECT 0x0E
#define ST25TB_CMD_COMPLETION 0x0F

#define ST25TB_ADDRESS_OTP 0xFF

int8_t st25tb_cmd_initiate(uint8_t *chip_id);
int8_t st25tb_cmd_select(const uint8_t chip_id);
int8_t st25tb_cmd_reset_to_inventory();
int8_t st25tb_cmd_write_block(const uint8_t address, const uint32_t block, uint8_t timeout_ms);
int8_t st25tb_cmd_get_uid(uint64_t *uid);
int8_t st25tb_cmd_read_block(uint8_t address, uint32_t *block);
int8_t st25tb_cmd_write_block_(const uint8_t address, const uint32_t block);