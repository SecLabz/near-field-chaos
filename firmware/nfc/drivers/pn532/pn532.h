#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pn532.h"
#include "pn532_spi.h"

#define PN532_IC 0x32

#define PN532_PREAMBLE (0x00)
#define PN532_STARTCODE1 (0x00)
#define PN532_STARTCODE2 (0xFF)
#define PN532_POSTAMBLE (0x00)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION (0x02)
#define PN532_COMMAND_GETGENERALSTATUS (0x04)
#define PN532_COMMAND_READREGISTER (0x06)
#define PN532_COMMAND_WRITEREGISTER (0x08)
#define PN532_COMMAND_READGPIO (0x0C)
#define PN532_COMMAND_WRITEGPIO (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE (0x10)
#define PN532_COMMAND_SETPARAMETERS (0x12)
#define PN532_COMMAND_SAMCONFIGURATION (0x14)
#define PN532_COMMAND_POWERDOWN (0x16)
#define PN532_COMMAND_RFCONFIGURATION (0x32)
#define PN532_COMMAND_RFREGULATIONTEST (0x58)
#define PN532_COMMAND_INJUMPFORDEP (0x56)
#define PN532_COMMAND_INJUMPFORPSL (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET (0x4A)
#define PN532_COMMAND_INATR (0x50)
#define PN532_COMMAND_INPSL (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU (0x42)
#define PN532_COMMAND_INDESELECT (0x44)
#define PN532_COMMAND_INRELEASE (0x52)
#define PN532_COMMAND_INSELECT (0x54)
#define PN532_COMMAND_INAUTOPOLL (0x60)
#define PN532_COMMAND_TGINITASTARGET (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES (0x92)
#define PN532_COMMAND_TGGETDATA (0x86)
#define PN532_COMMAND_TGSETDATA (0x8E)
#define PN532_COMMAND_TGSETMETADATA (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET (0x4B)

#define PN532_HOSTTOPN532 (0xD4)
#define PN532_PN532TOHOST (0xD5)

#define PN532_ACK_WAIT_TIME (10) // ms, timeout of waiting for ACK

#define PN532_INVALID_ACK (-1)
#define PN532_TIMEOUT (-2)
#define PN532_INVALID_FRAME (-3)
#define PN532_NO_SPACE (-4)

// Register addresses
#define PN53X_REG_Control_switch_rng 0x6106
#define PN53X_REG_CIU_Mode 0x6301
#define PN53X_REG_CIU_TxMode 0x6302
#define PN53X_REG_CIU_RxMode 0x6303
#define PN53X_REG_CIU_TxControl 0x6304
#define PN53X_REG_CIU_TxAuto 0x6305
#define PN53X_REG_CIU_TxSel 0x6306
#define PN53X_REG_CIU_RxSel 0x6307
#define PN53X_REG_CIU_RxThreshold 0x6308
#define PN53X_REG_CIU_Demod 0x6309
#define PN53X_REG_CIU_FelNFC1 0x630A
#define PN53X_REG_CIU_FelNFC2 0x630B
#define PN53X_REG_CIU_MifNFC 0x630C
#define PN53X_REG_CIU_ManualRCV 0x630D
#define PN53X_REG_CIU_TypeB 0x630E
#define PN53X_REG_CIU_Control 0x633C
#define PN53X_REG_CIU_Status2 0x6338

#define PN53X_REG_CIU_CWGsP 0x6318
#define PN53X_REG_CIU_ModGsP 0x6319
#define PN53X_REG_CIU_BitFraming 0x633D

#define SYMBOL_TX_LAST_BITS 0x07

// TX_FRAMING bits explanation:
//   00 : ISO/IEC 14443A/MIFARE and Passive Communication mode 106 kbit/s
//   01 : Active Communication mode
//   10 : FeliCa and Passive Communication mode at 212 kbit/s and 424 kbit/s
//   11 : ISO/IEC 14443B
#define SYMBOL_TX_FRAMING 0x03

// RX_FRAMING follow same scheme than TX_FRAMING
#define SYMBOL_RX_FRAMING 0x03

#define SYMBOL_TX_CRC_ENABLE 0x80
#define SYMBOL_TX_SPEED 0x70
#define SYMBOL_RX_SPEED 0x70
#define SYMBOL_RX_CRC_ENABLE 0x80
#define SYMBOL_RX_NO_ERROR 0x08
#define SYMBOL_RX_MULTIPLE 0x04

//   PN53X_REG_CIU_Status2
#define SYMBOL_MF_CRYPTO1_ON 0x08

//   PN53X_REG_CIU_ManualRCV
#define SYMBOL_PARITY_DISABLE 0x10

#define SYMBOL_INITIATOR 0x10

#define SYMBOL_FORCE_100_ASK 0x40

// RF communication
#define RFConfiguration 0x32
// Radio Field Configure Items           // Configuration Data length
#define RFCI_FIELD 0x01        //  1
#define RFCI_RETRY_SELECT 0x05 //  3

int16_t pn532_sam_configuration();
int16_t pn532_read_register(uint16_t reg, uint8_t *val);
int16_t pn532_write_register(uint16_t reg, uint8_t val);
int16_t pn532_write_register_with_mask(uint16_t reg, const uint8_t mask, const uint8_t val);
int16_t pn532_rf_timings(uint8_t atr_res_timeout, uint8_t reply_timeout);
int pn532_st25tb_init();
int pn532_st25tb_transceive_bytes(uint8_t *tx_buffer, size_t tx_length, uint8_t *rx_buffer, size_t rx_length, uint8_t timeout_ms);
void pn532_reset();
int pn532_rf_enable_discard_response(bool enable);
void pn532_rf_enable(bool enable);
void pn532_rf_tearoff_reset();
int16_t pn532_init_as_target();
int16_t pn532_get_data(uint8_t *buf, uint8_t len);
int pn532_set_data(const uint8_t *data, uint8_t len);
int pn532_in_release(const uint8_t target);
int pn532_status_as_target();