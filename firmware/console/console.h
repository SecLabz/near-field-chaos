#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "console_commands.h"
#include "../utils/task.h"
#include "../utils/utils.h"

#define CONSOLE_LINE_LENGTH 200
#define CONSOLE_LINE_BUFFER_LENGTH CONSOLE_LINE_LENGTH + 1

size_t console_append_char(char *buffer, char c);
size_t console_delete_char(char *buffer);
size_t console_clear_buffer(char *buffer);
void console_flush_input();
void console_start();