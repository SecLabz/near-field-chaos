#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include "st25tb_cmd.h"
#include "st25tb.h"
#include "../led/ws2812_led.h"
#include "../nfc/devices.h"

#define RESET "\033[0m"
#define BOLD "\033[01m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define GREEN "\033[32m"

#define IS_ONE_BIT(value, index) value & ((uint32_t)1 << index)
#define IS_ZERO_BIT(value, index) !(IS_ONE_BIT(value, index))

void st25tb_tear_off_init_start_offset();
uint32_t st25tb_tear_off_next_value(uint32_t current_value, bool randomness);
int8_t st25tb_tear_off_write_block(const uint8_t block_address, const uint32_t block_value, const int tear_off_us, bool init);
int8_t st25tb_tear_off_read_block(const uint8_t block_address, uint32_t *block_value);
int8_t st25tb_tear_off_retry_write_verify(const uint8_t block_address, uint32_t target_value, uint32_t max_try_count, int sleep_time_ms, uint32_t *new_value);
int8_t st25tb_tear_off_is_consolidated(const uint8_t block_address, uint32_t value, int repeat_read, int sleep_time_ms, uint32_t *read_value);
int8_t st25tb_tear_off_consolidate_block(const uint8_t block_address, uint32_t current_value, uint32_t target_value, uint32_t *read_back_value);
void st25tb_tear_off_log(int tear_off_us, char *color, uint32_t value);
void st25tb_tear_off_adjust_timing(int *tear_off_us, uint32_t tear_off_adjustment_us);
int8_t st25tb_tear_off(const int8_t block_address, uint32_t target_value, uint32_t tear_off_adjustment_us, uint32_t write_passes);