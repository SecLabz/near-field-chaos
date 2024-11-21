#pragma once
#include "pico/stdlib.h"
#include "../nfc/nfc.h"
#include "../st25tb/st25tb.h"
#include "../st25tb/st25tb_tag_storage.h"
#include "../led/ws2812_led.h"
#include "../console/console.h"
#include "t4t_ce.h"

// SRAM address to persist data between resets
#define MODE ((volatile uint32_t *)0x2003FFF8)

#define MODE_CONSOLE 0
#define MODE_RESTORE 1
#define MODE_LEARN 2
#define MODE_EMULATOR 3

void standalone_mode_learn_loop();
void standalone_mode_restore_loop(struct st25tb_tag *tag_to_write);
void standalone_mode_emulator_loop();
void standalone_mode_start();