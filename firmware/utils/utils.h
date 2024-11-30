#pragma once
#include <stdio.h>
#include <strings.h>
#include "pico/stdlib.h"

int8_t str_to_uint32(const char *str, uint32_t *out, const bool hex);
int8_t any_str_to_uint32(const char *str, uint32_t *out);
uint64_t convert_to_uint64(uint8_t array[8]);
void reverse_bits(uint8_t *buf, size_t size);