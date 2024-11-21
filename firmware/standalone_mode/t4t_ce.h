#pragma once
#include <stdio.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "../st25tb/st25tb_common.h"
#include "../nfc/drivers/pn532/pn532_spi.h"

void t4t_ce_emulate();
uint8_t t4t_ce_set_tag(struct st25tb_tag *t);
uint8_t t4t_ce_get_tag(struct st25tb_tag *t);
bool t4t_ce_tag_written();