#pragma once
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "pico/multicore.h"
#include "hardware/flash.h"
#include "st25tb_common.h"

// Operations (P/E cycles) on flash are one a whole sector with the pico sdk (no built-in partial update)
// A sector is 4096 bytes (FLASH_SECTOR_SIZE)
// A st25tb_tag is 76 bytes (TODO: double check, size could be wrong) (16*4 bytes for blocks, 2*4 bytes for uid, 4 byte for otp)
// A sector can contain approximately 53 tags (4096 / 76)
//
// The latest sector is used and structured like this
// 
// TODO: rotate used sectors to limit wear-out ?
#define FLASH_STORAGE_SIZE FLASH_SECTOR_SIZE * 1
#define FLASH_STORAGE_OFFSET ((2048 * 1024) - FLASH_STORAGE_SIZE)
#define FLASH_STORAGE_ADDRESS (XIP_BASE + FLASH_STORAGE_OFFSET)

#define TAG_STORAGE_MAXIMUM_CAPACITY 50
#define TAG_STORAGE_INITIALISED_FLAG_ADDRESS FLASH_STORAGE_ADDRESS
#define TAG_STORAGE_INITIALISED_FLAG 0xDEADBEEF
#define TAG_STORAGE_IS_INITIALISED() *((uint32_t *)TAG_STORAGE_INITIALISED_FLAG_ADDRESS) == TAG_STORAGE_INITIALISED_FLAG
#define TAG_STORAGE_ADDRESS FLASH_STORAGE_ADDRESS + 4

void st25tb_tag_storage_init(bool force);
uint8_t st25tb_tag_storage_save(const struct st25tb_tag *tag);
bool st25tb_tag_storage_exists(const uint64_t uid);
uint8_t st25tb_tag_storage_load(const uint64_t uid, struct st25tb_tag *tag);
uint8_t st25tb_tag_storage_load_by_index(const uint8_t index, struct st25tb_tag *tag);
void st25tb_tag_storage_print_tags();
uint8_t st25tb_tag_storage_delete(const uint8_t index);