#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "pico/stdlib.h"

#define ST25TB_TAG_BYTES_SIZE 19 * 4

struct st25tb_tag
{
    uint32_t blocks[16];
    uint32_t otp;
    uint64_t uid;
};

int8_t st25tb_tag_parse_from_string(const char *str, struct st25tb_tag *tag);