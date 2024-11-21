#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../utils/utils.h"
#include "../st25tb/st25tb.h"
#include "../st25tb/st25tb_tear_off.h"
#include "../st25tb/st25tb_tag_storage.h"

struct console_command
{
    char *name;
    char *options;
    char *description;
    void (*handle_fn)(const int argc, char *argv[]);
};

void console_print_help(const int argc, char *argv[]);
void console_read_tag(const int argc, char *argv[]);
void console_read_tag_raw(const int argc, char *argv[]);
void console_write_tag_raw(const int argc, char *argv[]);
void console_write_tag_block(const int argc, char *argv[]);
void console_tear_off(const int argc, char *argv[]);
void console_print_saved_tags(const int argc, char *argv[]);
uint8_t console_load_tag_from_index(const int argc, char *argv[], struct st25tb_tag *tag);
void console_do_print_saved_tag(const int argc, char *argv[], bool raw);
void console_print_saved_tag(const int argc, char *argv[]);
int8_t tearoff(const int8_t block_address, uint32_t target_value, uint32_t tear_off_adjustment_us);

extern struct console_command console_commands[8];
