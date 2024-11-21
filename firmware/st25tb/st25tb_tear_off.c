#include "st25tb_tear_off.h"

extern bool task_kill_signal;
extern bool task_running;

const int TEAR_OFF_ADJUSTMENT_US = 25;

int tear_off_start_offset_us = 0;

void st25tb_tear_off_init_start_offset()
{
    if (CHIP == CHIP_TRF7970A)
    {
        tear_off_start_offset_us = 900;
    }
    else if (CHIP == CHIP_PN532)
    {
        tear_off_start_offset_us = 1400;
    }
}

uint32_t st25tb_tear_off_next_value(uint32_t current_value, bool randomness)
{
    uint32_t value = 0;
    int8_t index = 31;

    if (current_value < 0x0000FFFF)
    {
        return current_value - 1;
    }

    // Loop through each bit starting from the most significant bit (MSB) to the least significant bit (LSB)
    while (index >= 0)
    {
        // Find the first '1' in the current_value starting from the MSB
        if (value == 0 && IS_ONE_BIT(current_value, index))
        {
            // Create a number with a single zero at position index
            value = 0xFFFFFFFF >> (31 - index);
            index--;
        }

        // Once we've found the first '1', now look for the first '0' after that '1'
        if (value != 0 && IS_ZERO_BIT(current_value, index))
        {
            index++;
            // Mask with a number with ones until index starting from the least significant bit (LSB)
            value &= ~((uint32_t)1 << index);

            // Introduce some randomization in the flipping process
            if (randomness && value < 0xF0000000 && index > 1)
            {
                value ^= ((uint32_t)1 << (rand() % index));
            }
            return value;
        }

        index--;
    }
    return value > 0 ? value - 1 : 0;
}

int8_t st25tb_tear_off_write_block(const uint8_t block_address, const uint32_t block_value, const int tear_off_us, bool init)
{
    int result;
    uint8_t chip_id = 0;

    result = st25tb_cmd_write_block(block_address, block_value, 0);

    sleep_us(tear_off_us);

    nfc_rf_tearoff_reset();
    return result;
}

int8_t st25tb_tear_off_read_block(const uint8_t block_address, uint32_t *block_value)
{
    int result;
    uint8_t chip_id = 0;
    if (st25tb_cmd_initiate(&chip_id) != 0)
    {
        printf("failed initiate\n");
        return -1;
    }
    if (st25tb_cmd_select(chip_id) != 0)
    {
        printf("failed select\n");

        return -1;
    }
    result = st25tb_cmd_read_block(block_address, block_value);
    return result;
}

int8_t st25tb_tear_off_retry_write_verify(const uint8_t block_address, uint32_t target_value, uint32_t max_try_count, int sleep_time_ms, uint32_t *read_back_value)
{
    int i = 0;

    while (*read_back_value != (target_value) && i < max_try_count)
    {
        st25tb_tear_off_write_block(block_address, target_value, 6000, false);
        sleep_ms(sleep_time_ms);
        st25tb_tear_off_read_block(block_address, read_back_value);
        sleep_ms(sleep_time_ms);
        i++;
    }

    if (*read_back_value == target_value)
    {
        printf("%sWriting:%s  0x%08X -> OK (count: %d)\n", BOLD, RESET, target_value, i);
        return 0;
    }

    // printf("%sWriting:%s  0x%08X -> KO (count: %d)\n", BOLD, RESET, target_value, i);

    return -1;
}

int8_t st25tb_tear_off_is_consolidated(const uint8_t block_address, uint32_t value, int repeat_read, int sleep_time_ms, uint32_t *read_value)
{
    int result;
    for (int i = 0; i < repeat_read; i++)
    {
        sleep_ms(sleep_time_ms);
        nfc_rf_enable(false);
        nfc_rf_enable(true);

        result = st25tb_tear_off_read_block(block_address, read_value);
        if (result != 0 || value != *read_value)
        {
            return -1;
        }
    }
    return 0;
}

int8_t st25tb_tear_off_consolidate_block(const uint8_t block_address, uint32_t current_value, uint32_t target_value, uint32_t *read_back_value)
{
    int8_t result;

    uint32_t consolidation_value;

    if (target_value <= 0xFFFFFFFD && current_value >= (target_value + 2))
    {
        consolidation_value = target_value + 2;
    }
    else
    {
        consolidation_value = current_value;
    }

    result = st25tb_tear_off_retry_write_verify(block_address, consolidation_value - 1, 30, 0, read_back_value);
    if (result != 0)
    {
        return -1;
    }

    if (*read_back_value != 0xFFFFFFFE || (*read_back_value == 0xFFFFFFFE && target_value == 0xFFFFFFFD))
    {
        result = st25tb_tear_off_retry_write_verify(block_address, consolidation_value - 2, 30, 0, read_back_value);
        if (result != 0)
        {
            return -1;
        }
    }

    if (result == 0 && target_value > 0xFFFFFFFD && *read_back_value > 0xFFFFFFFD)
    {
        result = st25tb_tear_off_is_consolidated(block_address, *read_back_value, 6, 0, read_back_value);
        if (result == 0)
        {
            printf("\n%sAlmost there ! %s\n", BOLD, RESET);
            result = st25tb_tear_off_is_consolidated(block_address, *read_back_value, 2, 2000, read_back_value);
            if (result != 0)
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

void st25tb_tear_off_log(int tear_off_us, char *color, uint32_t value)
{
    printf("%s%08X%s : %s", color, value, RESET, color);
    for (int i = 31; i >= 0; i--)
    {
        printf("%c", (value & (1 << i)) ? '1' : '0');
    }
    printf("%s : %02d us\n", RESET, tear_off_us);
}

void st25tb_tear_off_adjust_timing(int *tear_off_us, uint32_t tear_off_adjustment_us)
{
    if (*tear_off_us > tear_off_start_offset_us)
    {
        *tear_off_us = *tear_off_us - tear_off_adjustment_us;
    }
}

int8_t st25tb_tear_off(const int8_t block_address, uint32_t target_value, uint32_t tear_off_adjustment_us, uint32_t write_passes)
{
    int result;
    int count = 0;
    bool led_on;
    bool trigger = true;

    uint32_t read_value;
    uint32_t current_value;
    uint32_t last_consolidated_value;
    uint32_t tear_off_value;

    int current_pass = 0;

    st25tb_tear_off_init_start_offset();

    int tear_off_us = tear_off_start_offset_us;
    if (tear_off_adjustment_us == 0)
    {
        tear_off_adjustment_us = TEAR_OFF_ADJUSTMENT_US;
    }

    if (nfc_st25tb_init() != 0)
    {
        return -1;
    }

    result = st25tb_tear_off_read_block(block_address, &current_value);
    if (result != 0)
    {
        printf("Tag not found\n");
        return -1;
    }

    tear_off_value = st25tb_tear_off_next_value(current_value, false);
    printf(" Target block: %d\n", block_address);
    printf("Current value: 0x%08X\n", current_value);
    printf(" Target value: 0x%08X\n", target_value);
    printf("Adjustment us: %d\n", tear_off_adjustment_us);
    printf("Write passes : %d\n\n", write_passes);

    if (tear_off_value == 0)
    {
        printf("Tear off technique not possible.\n");
        return -1;
    }

    while (true)
    {
        // Fail safe
        if (tear_off_value < 0x00001000)
        {
            printf("Stopped. Safety first !");
            return -1;
        }

        // Tear off write
        st25tb_tear_off_write_block(block_address, tear_off_value, tear_off_us, false);

        // Read back potentially new value
        result = st25tb_tear_off_read_block(block_address, &read_value);
        if (result != 0)
        {
            continue;
        }

        // Act
        if (read_value > current_value)
        {
            if (read_value >= 0xFFFFFFFE ||
                (read_value - 2) >= target_value ||
                read_value != last_consolidated_value ||
                ((read_value & 0xF0000000) > (current_value & 0xF0000000)))
            {
                result = st25tb_tear_off_consolidate_block(block_address, read_value, target_value, &current_value);
                if (result == 0 && current_value == target_value)
                {
                    return 0;
                }
                st25tb_tear_off_log(tear_off_us, GREEN, read_value);
                if (read_value != last_consolidated_value)
                {
                    st25tb_tear_off_adjust_timing(&tear_off_us, tear_off_adjustment_us);
                }
                last_consolidated_value = read_value;
                tear_off_value = st25tb_tear_off_next_value(current_value, false);
                trigger = true;
            }
        }
        else if (read_value == tear_off_value)
        {
            if (trigger)
            {
                tear_off_value = st25tb_tear_off_next_value(tear_off_value, true);
                trigger = false;
            }
            else
            {
                tear_off_value = st25tb_tear_off_next_value(read_value, false);
                trigger = true;
            }
            current_value = read_value;
            st25tb_tear_off_log(tear_off_us, BLUE, read_value);
            st25tb_tear_off_adjust_timing(&tear_off_us, tear_off_adjustment_us);
        }
        else if (read_value < tear_off_value)
        {
            tear_off_value = st25tb_tear_off_next_value(read_value, false);
            current_value = read_value;
            trigger = true;
            st25tb_tear_off_log(tear_off_us, RED, read_value);
            st25tb_tear_off_adjust_timing(&tear_off_us, tear_off_adjustment_us);
        }

        if (++current_pass >= write_passes)
        {
            current_pass = 0;
            tear_off_us += 1;
        }

        if (count++ > 30)
        {
            if (led_on)
            {
                led_yellow();
            }
            else
            {
                led_off();
            }
            led_on = !led_on;
            count = 0;
        }

        if (task_kill_signal)
        {
            task_running = false;
            return 1;
        }
    }

    return -1;
}