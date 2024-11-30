#include "st25tb.h"

void st25tb_tag_print(const struct st25tb_tag *tag)
{
    uint8_t i;

    printf("UID: %016" PRIX64 "\n\n", tag->uid);

    printf("+---------------+----------+--------------------+\n");
    printf("| BLOCK ADDRESS |  VALUE   |    DESCRIPTION     |\n");
    printf("+---------------+----------+--------------------+\n");

    for (i = 0; i < 16; i++)
    {

        if (i == 2)
        {
            printf("|     %03d       | %08" PRIX32 " |   Lockable EEPROM  |\n", i, tag->blocks[i]);
        }
        else if (i == 5)
        {
            printf("|     %03d       | %08" PRIX32 " |     Count down     |\n", i, tag->blocks[i]);
        }
        else if (i == 6)
        {
            printf("|     %03d       | %08" PRIX32 " |       counter      |\n", i, tag->blocks[i]);
        }
        else if (i == 11)
        {
            printf("|     %03d       | %08" PRIX32 " |   Lockable EEPROM  |\n", i, tag->blocks[i]);
        }
        else
        {
            printf("|     %03d       | %08" PRIX32 " |                    |\n", i, tag->blocks[i]);
        }
        if (i == 4 || i == 6 || i == 15)
        {
            printf("+---------------+----------+--------------------+\n");
        }
    }

    printf("|     %03d       | %08" PRIX32 " |   System OTP bits  |\n", ST25TB_ADDRESS_OTP, tag->otp);
    printf("+---------------+----------+--------------------+\n");
}

void st25tb_tag_print_raw(const struct st25tb_tag *tag)
{
    uint8_t i;

    for (i = 0; i < 16; i++)
    {
        printf("%08" PRIX32, tag->blocks[i]);
    }

    printf("%08" PRIX32 "", tag->otp);
    printf("%" PRIX64 "", tag->uid);
}

int8_t st25tb_tag_read(struct st25tb_tag *tag)
{
    int8_t result;
    uint8_t i, chip_id;

    if (st25tb_cmd_initiate(&chip_id) != 0)
    {
        return -1;
    }

    if (st25tb_cmd_select(chip_id) != 0)
    {
        return -1;
    }

    result = st25tb_cmd_get_uid(&tag->uid);
    if (result != 0)
    {
        return -1;
    }

    for (i = 0; i < 16; i++)
    {
        result = st25tb_cmd_read_block(i, &tag->blocks[i]);
        if (result != 0)
        {
            return -1;
        }
    }

    result = st25tb_cmd_read_block(ST25TB_ADDRESS_OTP, &tag->otp);
    if (result != 0)
    {
        return -1;
    }

    st25tb_cmd_reset_to_inventory();

    return 0;
}

int8_t st25tb_tag_write_block(const uint8_t address, const uint32_t block, bool check)
{
    int8_t result;
    uint8_t chip_id;
    uint32_t read_block;

    result = st25tb_cmd_initiate(&chip_id);
    if (result != 0)
    {
        return -1;
    }

    result = st25tb_cmd_select(chip_id);
    if (result != 0)
    {
        return -1;
    }

    result = st25tb_cmd_write_block(address, block, ST25TB_TRANSCEIVE_TIMEOUT_MS);
    if (result != 0)
    {
        return -1;
    }

    if (check)
    {
        sleep_ms(6);

        result = st25tb_cmd_read_block(address, &read_block);
        if (result != 0)
        {
            return -1;
        }

        st25tb_cmd_reset_to_inventory();

        if (read_block != block)
        {
            return -1;
        }
    }

    return 0;
}

int8_t st25tb_tag_write(struct st25tb_tag *tag)
{
    int8_t result;
    uint8_t chip_id, i;

    nfc_st25tb_init();

    result = st25tb_cmd_initiate(&chip_id);
    if (result != 0)
    {
        return -1;
    }

    result = st25tb_cmd_select(chip_id);
    if (result != 0)
    {
        return -1;
    }

    for (i = 0; i < 16; i++)
    {
        result = st25tb_cmd_write_block(i, tag->blocks[i], ST25TB_TRANSCEIVE_TIMEOUT_MS);
        if (result != 0)
        {
            return -1;
        }
        sleep_ms(10); // tW, programming time for write
    }

    // result = st25tb_cmd_write_block(ST25TB_ADDRESS_OTP, tag->otp, ST25TB_TRANSCEIVE_TIMEOUT_MS);
    // if (result != 0)
    // {
    //     return -1;
    // }

    sleep_ms(6); // tW, programming time for write

    st25tb_cmd_reset_to_inventory();

    return 0;
}