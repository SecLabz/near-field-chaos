#include "t4t_ce.h"

typedef enum
{
    NONE,
    CC,
    NDEF
} tag_file; // CC ... Compatibility Container

#define NDEF_MAX_LENGTH 128
// Command APDU
#define C_APDU_CLA 0
#define C_APDU_INS 1  // instruction
#define C_APDU_P1 2   // parameter 1
#define C_APDU_P2 3   // parameter 2
#define C_APDU_LC 4   // length command
#define C_APDU_DATA 5 // data

bool tagWrittenByInitiator;

uint8_t ndef_buf[600];
uint8_t t4t_ce_recv_ndef_buffer[500];
char write_tag_str[500];
bool tag_written = false;

struct st25tb_tag tag = {
    .blocks = {
        // Lockable EEPROM
        0x00000000,
        0xCAFEBABE,
        0xDEADBEEF,
        0x00000000,
        0x00000000,

        // Count down counter
        0xfffffffe,
        0xfffffffe,

        // Lockable EEPROM
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000},
    .otp = 0xffffffff,
    .uid = 0xD0023370483E85EF,
};

void make_tag_ndef(struct st25tb_tag *tag, uint8_t *ndef_buffer, uint8_t *len)
{
    uint32_t address = 0;
    uint8_t i, j;

    ndef_buffer[address++] = 0x00;
    ndef_buffer[address++] = 0x00; // size to replace
    ndef_buffer[address++] = 0xD1; // MB, ME, SR, TNF=”NFC Forum well-known type”
    ndef_buffer[address++] = 0x1;  // Type length
    ndef_buffer[address++] = 0;    // ndef legnth  to replace  //3 + text_length; // Payload length
    ndef_buffer[address++] = 0x54; //  Type - "T"
    ndef_buffer[address++] = 0x2;  // Status byte - Length of IANA lang code
    ndef_buffer[address++] = 0x65; // language code "e"
    ndef_buffer[address++] = 0x6E; // language code "n"

    char utf8_string[9]; // 8 char + null

    ndef_buffer[address++] = 0xA; // \n
    for (j = 0; j < 16; j++)
    {
        snprintf(utf8_string, sizeof(utf8_string), "%08" PRIX32, tag->blocks[j]);
        for (i = 0; i < 8; i++)
        {
            ndef_buffer[address++] = utf8_string[i];
        }
        ndef_buffer[address++] = 0xa; // \n
    }

    snprintf(utf8_string, sizeof(utf8_string), "%08" PRIX32, tag->otp);
    for (i = 0; i < 8; i++)
    {
        ndef_buffer[address++] = utf8_string[i];
    }
    ndef_buffer[address++] = 0xa; // \n

    snprintf(utf8_string, sizeof(utf8_string), "%08" PRIX32, (uint32_t)(tag->uid >> 32));
    for (i = 0; i < 8; i++)
    {
        ndef_buffer[address++] = utf8_string[i];
    }
    ndef_buffer[address++] = 0xa; // \n

    snprintf(utf8_string, sizeof(utf8_string), "%08" PRIX32, (uint32_t)(tag->uid & 0xFFFFFFFF));
    for (i = 0; i < 8; i++)
    {
        ndef_buffer[address++] = utf8_string[i];
    }
    ndef_buffer[address++] = 0xa; // \n

    ndef_buffer[1] = address - 2; // ndef file size
    ndef_buffer[4] = address - 6; // ndef message payload size - start after type T

    *len = address;
}

uint8_t cc_buffer[26] = {
    0x00, 0x0F, // CC Len - 7 (fixed) + #Files * 8 (i.e. 2 files, 7+2*8=23) 1 Files (1 NDEF)
    0x20,       // MAP Ver 2.0
    0x00, 0x60, // MLe
    0x00, 0x58, // MLc - problem with number above 0x58, fifo troubles..

    0x04,       // T (NDEF File)
    0x06,       // L
    0xE1, 0x04, // File ID
    0x01, 0xF4, // Max NDEF - 500 bytes
    0x00,       // R
    0x00,       // W - 0x00 (write capability available), 0xFF (read-only)

    0x90, 0x00 // end of apdu
};

void t4t_ce_state_machine(uint8_t *rx_buffer, uint8_t *rx_length, uint8_t *tx_buffer, uint8_t *tx_length)
{
    uint8_t i, j, address, start, count;

    // Select app
    if (*rx_length >= 13 &&
        rx_buffer[0] == 0x00 &&
        rx_buffer[1] == 0xA4 &&
        rx_buffer[2] == 0x04 &&
        rx_buffer[3] == 0x00 &&
        rx_buffer[5] == 0xD2 &&
        (rx_buffer[11] == 0x01 || rx_buffer[11] == 0x00))
    {
        tx_buffer[0] = 0x90;
        tx_buffer[1] = 0x00;
        *tx_length = 2;
    }
    // App does not exists
    else if (*rx_length >= 14 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xA4 &&
             rx_buffer[2] == 0x04 &&
             rx_buffer[3] == 0x00)
    {
        tx_buffer[0] = 0x6A;
        tx_buffer[1] = 0x82;
        *tx_length = 2;
    }
    // select cc file 03 00 A4 00 0C 02 E1 03 D2AF
    else if (*rx_length >= 8 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xA4 &&
             rx_buffer[2] == 0x00 &&
             rx_buffer[3] == 0x0C &&
             rx_buffer[4] == 0x02 &&
             rx_buffer[5] == 0xE1 &&
             rx_buffer[6] == 0x03)
    {
        tx_buffer[0] = 0x90;
        tx_buffer[1] = 0x00;
        *tx_length = 2;
    }
    // Read Binary cc file (Capability Container) 02 00 B0 00 00 0F 8E A6
    else if (*rx_length >= 5 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xB0 &&
             rx_buffer[2] == 0x00 &&
             rx_buffer[3] == 0x00 &&
             rx_buffer[4] == 0x0F)
    {
        for (int k = 0; k < 17; k++)
        {
            tx_buffer[k] = cc_buffer[k];
        }
        *tx_length = 17;
    }
    //  NDEF File Select 03 00 A4 00 0C 02 E104 6DDB
    else if (*rx_length >= 5 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xA4 &&
             rx_buffer[2] == 0x00 &&
             rx_buffer[3] == 0x0C &&
             rx_buffer[4] == 0x02 &&
             rx_buffer[5] == 0xE1 &&
             rx_buffer[6] == 0x04)
    {
        tx_buffer[0] = 0x90;
        tx_buffer[1] = 0x00;
        *tx_length = 2;
    }
    // Deselect ack
    else if (*rx_length == 2 && rx_buffer[0] == 0xC2)
    {
        tx_buffer[0] = 0xC2;
        *tx_length = 1;
    }
    // NDEF Read file
    else if (*rx_length >= 5 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xB0 &&
             rx_buffer[2] == 0x00)
    {

        start = rx_buffer[3];
        count = rx_buffer[4];

        // todo check start/count out of bound read

        printf("read start: %d count: %d\n", start, count);

        make_tag_ndef(&tag, (uint8_t *)&ndef_buf, &j);

        for (int k = 0; k < count; k++)
        {
            tx_buffer[k] = ndef_buf[start + k];
        }
        tx_buffer[count] = 0x90;
        tx_buffer[count + 1] = 0x00;

        *tx_length = count + 2;
    }
    // NDEF Write file
    else if (*rx_length >= 4 &&
             rx_buffer[0] == 0x00 &&
             rx_buffer[1] == 0xD6 &&
             rx_buffer[2] == 0x00)
    {
        bool error = false;

        start = rx_buffer[3];
        count = rx_buffer[4];

        for (i = 0; i < count; i++)
        {
            t4t_ce_recv_ndef_buffer[start + i] = rx_buffer[5 + i];
        }

        // todo check start/count out of bound write

        // printf("write start: %d count: %d\n", start, count);

        if (start == 0 && count == 2 &&
            t4t_ce_recv_ndef_buffer[1] != 0 &&    // check ndef file size not null
            t4t_ce_recv_ndef_buffer[2] == 0xD1 && // check MB, ME, SR, TNF=”NFC Forum well-known type”
            t4t_ce_recv_ndef_buffer[4] != 0)      // check payload length
        {
            int content_len = t4t_ce_recv_ndef_buffer[4] - 3;
            j = 0;
            for (size_t i = 0; i < content_len; i++)
            {
                if (t4t_ce_recv_ndef_buffer[i + 9] == 0xA) // skip \n
                {
                    continue;
                }
                sprintf(&write_tag_str[j++], "%c", t4t_ce_recv_ndef_buffer[i + 9]);
            }
            write_tag_str[j] = '\0';

            int8_t result = st25tb_tag_parse_from_string(write_tag_str, &tag);
            if (result != 0)
            {
                error = true;
            }
            else
            {
                tag_written = true;
            }
        }

        // 03 PCB
        // 00 Class byte
        // D6 Write instruction code
        // 00 00 Offset in the file selected
        // 0D Number of bytes of data
        // 00 00 Number of bytes of ndef message
        // D1 MB, ME, SR, TNF=”NFC Forum well-known type”
        // 01 Type length
        // 07 Payload length (minus payload type)
        // 54 Payload type
        // 02 65 6E 54 6F 74 6F payload
        // 9B5E CRC

        if (error)
        {
            tx_buffer[0] = 0x65;
            tx_buffer[1] = 0x81;
        }
        else
        {
            tx_buffer[0] = 0x90;
            tx_buffer[1] = 0x00;
        }

        *tx_length = 2;
    }
}

int8_t t4t_ce_set_tag(struct st25tb_tag *t)
{
    void *result;
    result = memcpy(&tag, t, sizeof(struct st25tb_tag));
    if (result == NULL)
    {
        return -1;
    }
    return 0;
}

int8_t t4t_ce_get_tag(struct st25tb_tag *t)
{
    void *result;
    result = memcpy(t, &tag, sizeof(struct st25tb_tag));
    if (result == NULL)
    {
        return -1;
    }
    return 0;
}

bool t4t_ce_tag_written()
{
    return tag_written;
}

void t4t_ce_emulate()
{
    uint8_t compatibility_container[] = {
        0, 0x0F,
        0x20,
        0, 0x54,
        0, 0xFF,
        0x04,                                                        // T
        0x06,                                                        // L
        0xE1, 0x04,                                                  // File identifier
        ((NDEF_MAX_LENGTH & 0xFF00) >> 8), (NDEF_MAX_LENGTH & 0xFF), // maximum NDEF file size
        0x00,                                                        // read access 0x0 = granted
        0x00                                                         // write access 0x0 = granted | 0xFF = deny
    };

    compatibility_container[14] = 0xFF;

    tagWrittenByInitiator = false;

    uint8_t rwbuf[128];
    uint8_t tx_buf[600];
    uint8_t tx_length;

    uint8_t sendlen;
    int16_t status;
    tag_file currentFile = NONE;
    uint16_t cc_size = sizeof(compatibility_container);
    bool runLoop = true;

    pn532_wakeup();
    pn532_sam_configuration();
    int res = pn532_init_as_target();
    if (res < 0)
    {
        return;
    }

    while (true)
    {

        status = pn532_get_data(rwbuf, sizeof(rwbuf));
        if (status < 0)
        {
            printf("tgGetData failed!\n");
            pn532_in_release(0);
            break;
        }

        // printf("recv: ");
        // for (int k = 0; k < status; k++)
        // {
        //     printf("%02X ", rwbuf[k]);
        // }
        // printf("\n");

        t4t_ce_state_machine(rwbuf, (uint8_t *)&status, tx_buf, &tx_length);

        // printf("send: ");
        // for (int k = 0; k < tx_length; k++)
        // {
        //     printf("%02X ", tx_buf[k]);
        // }
        // printf("\n");

        if (pn532_set_data(tx_buf, tx_length) != 0)
        {
            printf("t4t set data error\n");
            break;
        }
    }
}