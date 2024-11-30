#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../nfc/nfc.h"
#include "st25tb_common.h"

#include "st25tb_cmd.h"

int8_t st25tb_tag_read(struct st25tb_tag *tag);
void st25tb_tag_print(const struct st25tb_tag *tag);
void st25tb_tag_print_raw(const struct st25tb_tag *tag);
int8_t st25tb_tag_write(struct st25tb_tag *tag);
int8_t st25tb_tag_write_block(const uint8_t address, const uint32_t block, bool check);