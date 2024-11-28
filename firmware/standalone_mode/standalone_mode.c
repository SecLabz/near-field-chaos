#include "standalone_mode.h"

static struct st25tb_tag read_tag, stored_tag, emu_tag;

void standalone_mode_learn_loop()
{
    int result;
    nfc_rf_enable(false);
    nfc_rf_enable(true);

    result = st25tb_tag_read(&read_tag);
    if (result != 0)
    {
        return;
    }

    result = st25tb_tag_storage_save(&read_tag);
    if (result != 0)
    {
        led_red();
        sleep_ms(200);
        return;
    }

    while (true)
    {
        led_green();
        sleep_ms(200);
        led_blue();
        sleep_ms(200);
    }
}

void standalone_mode_restore_loop(struct st25tb_tag *tag_to_write)
{
    int result;
    nfc_st25tb_init();

    result = st25tb_tag_read(&read_tag);
    if (result != 0)
    {
        return;
    }

    if (tag_to_write == NULL)
    {
        result = st25tb_tag_storage_load(read_tag.uid, &stored_tag);
        if (result != 0)
        {
            led_red();
            sleep_ms(200);
            return;
        }
        tag_to_write = &stored_tag;
    }

    if (read_tag.blocks[5] < tag_to_write->blocks[5] && tearoff(5, tag_to_write->blocks[5], 25) != 0)
    {
        while (true)
        {
            led_red();
            sleep_ms(500);
            led_orange();
            sleep_ms(500);
        }
    }

    if (read_tag.blocks[6] < tag_to_write->blocks[6] && tearoff(6, tag_to_write->blocks[6], 25) != 0)
    {
        while (true)
        {
            led_red();
            sleep_ms(500);
            led_orange();
            sleep_ms(500);
        }
    }

    result = st25tb_tag_write(tag_to_write);
    if (result != 0)
    {
        led_red();
        sleep_ms(500);
        return;
    }

    while (true)
    {
        led_green();
        sleep_ms(200);
        led_orange();
        sleep_ms(200);
    }
}

void standalone_mode_emulator_loop()
{
    int result;

    if (t4t_ce_tag_written())
    {
        led_orange();
        t4t_ce_get_tag(&emu_tag);
        standalone_mode_restore_loop(&emu_tag);
        return;
    }
    led_purple();

    nfc_st25tb_init();

    result = st25tb_tag_read(&read_tag);
    if (result == 0)
    {
        t4t_ce_set_tag(&read_tag);
        led_green();
        sleep_ms(600);
        led_purple();
    }
    t4t_ce_emulate();
}

void standalone_mode_start()
{
    if (*MODE < 3)
    {
        (*MODE)++;
    }
    else
    {
        (*MODE) = 1;
    }

    while (true)
    {
        if (stdio_usb_connected())
        {
            led_white();
            console_start();
        }

        if (*MODE == MODE_LEARN)
        {
            led_blue();
            standalone_mode_learn_loop();
        }
        else if (*MODE == MODE_RESTORE)
        {
            led_orange();
            standalone_mode_restore_loop(NULL);
        }
        else if (*MODE == MODE_EMULATOR)
        {
            standalone_mode_emulator_loop();
        }
    }
}