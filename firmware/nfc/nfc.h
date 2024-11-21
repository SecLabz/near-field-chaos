#pragma once
#include <stdio.h>
#include "devices.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "nfc_status.h"
#include "devices.h"

int nfc_init();
void nfc_deinit();
void nfc_reset();
void nfc_rf_tearoff_reset();
void nfc_rf_enable(bool enable);
int nfc_st25tb_init();
int nfc_st25tb_transceive_bytes(uint8_t *tx_buffer, size_t tx_length, uint8_t *rx_buffer, size_t rx_length, uint8_t timeout_ms);