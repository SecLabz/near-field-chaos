#include "console_commands.h"

struct console_command console_commands[] = {
    {.name = "h", .options = "", .description = "print help", .handle_fn = console_print_help},
    {.name = "l", .options = "", .description = "list saved tags UIDs", .handle_fn = console_print_saved_tags},
    {.name = "p", .options = "[index]", .description = "print a saved tag", .handle_fn = console_print_saved_tag},
    {.name = "r", .options = "", .description = "read a st25tb tag", .handle_fn = console_read_tag},
    {.name = "read_tag_raw", .options = "", .description = "read and print a tag (raw format)", .handle_fn = console_read_tag_raw},
    {.name = "to", .options = "[address] [value]", .description = "write a counter value with tear off technique (max 0xFFFFFFFE), ex: to 5 FFFFFFFE", .handle_fn = console_tear_off},
    {.name = "wb", .options = "[address] [value]", .description = "write a st25tb block, ex: wb 7 DEADBEEF", .handle_fn = console_write_tag_block},
    {.name = "write_tag_raw", .options = "[value]", .description = "write tag (raw format)", .handle_fn = console_write_tag_raw},
};

static struct st25tb_tag tag_to_write, read_tag;

void console_print_help(const int argc, char *argv[])
{
    int i;
    int max_command_str_len = 0;
    int command_str_len = 0;
    int spaces_count = 0;

    printf("\nCommands:\n\n");
    for (i = 0; i < sizeof(console_commands) / sizeof(console_commands[0]); i++)
    {
        command_str_len = strlen(console_commands[i].name) + strlen(console_commands[i].options);
        if (strlen(console_commands[i].options) > 0)
        {
            command_str_len++;
        }
        if (command_str_len > max_command_str_len)
        {
            max_command_str_len = command_str_len;
        }
    }

    for (i = 0; i < sizeof(console_commands) / sizeof(console_commands[0]); i++)
    {
        command_str_len = strlen(console_commands[i].name) + strlen(console_commands[i].options);
        if (strlen(console_commands[i].options) > 0)
        {
            command_str_len++;
        }
        spaces_count = max_command_str_len - command_str_len;

        spaces_count++;
        printf(strlen(console_commands[i].options) > 0 ? "  \033[01m%s\033[0m %s%*c%s\n" : "  \033[01m%s\033[0m%s%*c%s\n", console_commands[i].name, console_commands[i].options, spaces_count, ' ', console_commands[i].description);
    }
}

void console_read_tag(const int argc, char *argv[])
{
    uint8_t result;
    struct st25tb_tag tag;

    if (nfc_st25tb_init() != 0)
    {
        printf("Error while reading tag (init chip failed)\n");
        return;
    }

    result = st25tb_tag_read(&tag);
    if (result != 0)
    {
        printf("Error while reading tag\n");
        return;
    }
    st25tb_tag_print(&tag);
}

void console_read_tag_raw(const int argc, char *argv[])
{
    uint8_t result;
    struct st25tb_tag tag;

    if (nfc_st25tb_init() != 0)
    {
        printf("ERROR:Error while reading tag (init chip failed)\r\n");
        return;
    }

    result = st25tb_tag_read(&tag);
    if (result != 0)
    {
        printf("ERROR:Error while reading tag\r\n");
        return;
    }
    printf("RESPONSE:");
    st25tb_tag_print_raw(&tag);
}

int8_t tearoff(const int8_t block_address, uint32_t target_value, uint32_t tear_off_adjustment_us)
{
    absolute_time_t start_time;
    absolute_time_t end_time;
    int64_t time_diff_us;
    int64_t time_diff_ms;
    int minutes;
    int seconds;
    start_time = get_absolute_time();
    int8_t ret = st25tb_tear_off(block_address, target_value, tear_off_adjustment_us, 0);

    end_time = get_absolute_time();

    time_diff_us = absolute_time_diff_us(start_time, end_time);
    time_diff_ms = time_diff_us / 1000;
    minutes = time_diff_ms / 60000;
    seconds = (time_diff_ms % 60000) / 1000;
    if (ret == 0)
    {
        printf("Tear off success, job done in %dm %02ds\r\n", minutes, seconds);
    }
    else if (ret == 1)
    {
        printf("Tear off stopped\r\n");
    }
    else
    {
        printf("ERROR:Tear off failed\r\n");
    }
    led_white();
    return ret;
}

void console_write_tag_block(const int argc, char *argv[])
{
    uint8_t result;
    uint32_t address, value;

    if (argc != 3)
    {
        printf("Wrong arguments. \nExample: wb 5 DEADBEEF\n");
        return;
    }

    result = any_str_to_uint32(argv[1], &address);
    if (result != 0)
    {
        printf("Error while parsing block address\n");
        return;
    }

    result = str_to_uint32(argv[2], &value, true);
    if (result != 0)
    {
        printf("Error while parsing block value\n");
        return;
    }

    if (nfc_st25tb_init() != 0)
    {
        printf("Error while writing tag (init chip failed)\n");
        return;
    }

    result = st25tb_tag_write_block((uint8_t)address, value, true);
    if (result != 0)
    {
        printf("Error while writing tag\n");
        return;
    }

    printf("Done\n");
}

void console_write_tag_raw(const int argc, char *argv[])
{
    uint8_t result;
    uint32_t address, value;

    if (argc != 2)
    {
        printf("ERROR:Wrong arguments.\n");
        return;
    }

    result = st25tb_tag_parse_from_string(argv[1], &tag_to_write);
    if (result != 0)
    {
        printf("ERROR:Bad tag format\n");
        return;
    }

    nfc_st25tb_init();

    result = st25tb_tag_read(&read_tag);
    if (result != 0)
    {
        printf("ERROR:Error while reading tag\n");
        return;
    }

    uint32_t tear_off_adjustment_us = 0;
    if (tag_to_write.blocks[5] > read_tag.blocks[5])
    {
        result = tearoff(5, tag_to_write.blocks[5], tear_off_adjustment_us);
        if (result != 0)
        {
            printf("ERROR:Tear off write error on counter at block 5\n");
            return;
        }
    }

    if (tag_to_write.blocks[6] > read_tag.blocks[6])
    {
        result = tearoff(6, tag_to_write.blocks[6], tear_off_adjustment_us);
        if (result != 0)
        {
            printf("ERROR:Tear off write error on counter at block 6\n");
            return;
        }
    }

    result = st25tb_tag_write(&tag_to_write);
    if (result != 0)
    {
        printf("ERROR:Error while writing tag content\n");
        return;
    }

    printf("RESPONSE:Tag successfully written\n");
}

void console_tear_off(const int argc, char *argv[])
{
    uint8_t result;
    uint32_t address;
    uint32_t value;
    uint32_t tear_off_adjustment_us = 0;
    uint32_t write_passes = 0;
    struct st25tb_tag tag;
    absolute_time_t start_time;
    absolute_time_t end_time;
    int64_t time_diff_us;
    int64_t time_diff_ms;
    int minutes;
    int seconds;

    if (argc != 3 && argc != 4 && argc != 5)
    {
        printf("Wrong arguments. \nExample: to 5 FFFFFFFE\n");
        return;
    }

    result = any_str_to_uint32(argv[1], &address);
    if (result != 0)
    {
        printf("Error while parsing block address\n");
        return;
    }

    result = str_to_uint32(argv[2], &value, true);
    if (result != 0)
    {
        printf("Error while parsing block value\n");
        return;
    }

    if (argc == 4)
    {
        result = any_str_to_uint32(argv[3], &tear_off_adjustment_us);
        if (result != 0)
        {
            printf("Error while parsing adjustment time\n");
            return;
        }
    }

    if (argc == 5)
    {
        result = any_str_to_uint32(argv[4], &write_passes);
        if (result != 0)
        {
            printf("Error while parsing write passes\n");
            return;
        }
    }

    printf("\n");
    printf("START TEAR OFF\n\n");

    start_time = get_absolute_time();

    result = st25tb_tear_off(address, value, tear_off_adjustment_us, write_passes);

    end_time = get_absolute_time();

    time_diff_us = absolute_time_diff_us(start_time, end_time);
    time_diff_ms = time_diff_us / 1000;
    minutes = time_diff_ms / 60000;
    seconds = (time_diff_ms % 60000) / 1000;

    printf("\n");
    if (result != 0)
    {
        printf("Error while glitching the tag\n\n");
        return;
    }

    printf("\n");

    nfc_st25tb_init();
    result = st25tb_tag_read(&tag);
    if (result != 0)
    {
        printf("Error while reading tag\n");
    }
    else
    {
        st25tb_tag_print(&tag);
    }

    printf("\nJob done in %dm %02ds\n\n", minutes, seconds);
}

void console_print_saved_tags(const int argc, char *argv[])
{
    st25tb_tag_storage_print_tags();
}

uint8_t console_load_tag_from_index(const int argc, char *argv[], struct st25tb_tag *tag)
{
    uint8_t result;
    uint32_t index;

    if (argc != 2)
    {
        printf("Wrong arguments\n");
        return -1;
    }

    result = any_str_to_uint32(argv[1], &index);
    if (result != 0)
    {
        printf("Error while parsing index\n");
        return -1;
    }

    result = st25tb_tag_storage_load_by_index((uint8_t)index, tag);
    if (result != 0)
    {
        printf("Tag not found\n");
        return -1;
    }

    return 0;
}

void console_do_print_saved_tag(const int argc, char *argv[], bool raw)
{
    uint8_t result;
    struct st25tb_tag tag;

    result = console_load_tag_from_index(argc, argv, &tag);
    if (result != 0)
    {
        return;
    }

    if (raw)
    {
        st25tb_tag_print_raw(&tag);
    }
    else
    {
        st25tb_tag_print(&tag);
    }
}

void console_print_saved_tag(const int argc, char *argv[])
{
    console_do_print_saved_tag(argc, argv, false);
}