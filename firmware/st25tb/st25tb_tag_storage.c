#include "st25tb_tag_storage.h"
#include <hardware/sync.h>

void st25tb_tag_storage_init(bool force)
{
    if (TAG_STORAGE_IS_INITIALISED() && !force)
    {
        return;
    }

    uint8_t page_buffer[FLASH_PAGE_SIZE];
    uint32_t ints;

    *(uint32_t *)page_buffer = TAG_STORAGE_INITIALISED_FLAG;

    // TODO: memset ?
    for (int i = 4; i < FLASH_PAGE_SIZE; i++)
    {
        page_buffer[i] = 0xFF;
    }

    ints = save_and_disable_interrupts();

    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_STORAGE_OFFSET, page_buffer, FLASH_PAGE_SIZE);

    restore_interrupts(ints);
}

void st25tb_tag_storage_print_tags()
{
    uint8_t count = 0;
    struct st25tb_tag *stored_tag = (struct st25tb_tag *)(FLASH_STORAGE_ADDRESS + 4);
    while (stored_tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (count == 0)
        {
            printf("+---------+-----------------------+\n");
            printf("|  INDEX  |          UID          |\n");
            printf("+---------+-----------------------+\n");
        }
        printf("|    %02d   |   %" PRIX64 "    |\n", count, stored_tag->uid);
        stored_tag = stored_tag + 1;
        count++;
    }

    if (count > 0)
    {
        printf("+---------+-----------------------+\n");
    }
    else
    {
        printf("No tag found.\n");
    }
}

uint8_t st25tb_tag_storage_save(const struct st25tb_tag *tag)
{
    uint32_t ints, count;
    bool found = false;
    uint8_t sector_buffer[FLASH_SECTOR_SIZE];
    struct st25tb_tag *stored_tag;

    memcpy(sector_buffer, (void *)FLASH_STORAGE_ADDRESS, sizeof(sector_buffer));
    stored_tag = (struct st25tb_tag *)(sector_buffer + 4);

    count = 0;
    while (stored_tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (stored_tag->uid == tag->uid)
        {
            memcpy(stored_tag, tag, sizeof(struct st25tb_tag));
            found = true;
            break;
        }
        stored_tag = stored_tag + 1;
        count++;
    }

    if (count > TAG_STORAGE_MAXIMUM_CAPACITY)
    {
        return -1;
    }

    if (!found)
    {
        memcpy(stored_tag, tag, sizeof(struct st25tb_tag));
    }

    // Stop other core to avoid losing console uart
    uint core = get_core_num();
    if (core == 1)
    {
        multicore_lockout_start_blocking();
    }

    ints = save_and_disable_interrupts();

    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_STORAGE_OFFSET, sector_buffer, FLASH_SECTOR_SIZE);

    restore_interrupts(ints);

    if (core == 1)
    {
        multicore_lockout_end_blocking();
    }

    return 0;
}

uint8_t st25tb_tag_storage_load_by_index(const uint8_t index, struct st25tb_tag *tag)
{
    uint8_t count = 0;
    struct st25tb_tag *stored_tag = (struct st25tb_tag *)(FLASH_STORAGE_ADDRESS + 4);
    while (stored_tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (index == count)
        {
            memcpy(tag, stored_tag, sizeof(struct st25tb_tag));
            return 0;
        }
        stored_tag = stored_tag + 1;
        count++;
    }
    return -1;
}

uint8_t st25tb_tag_storage_delete(const uint8_t index)
{
    uint32_t ints, count;
    bool found = false;
    uint8_t sector_buffer[FLASH_SECTOR_SIZE];
    struct st25tb_tag *stored_tag, *new_stored_tag;

    memcpy(sector_buffer, (void *)FLASH_STORAGE_ADDRESS, 4);
    for (int i = 4; i < FLASH_SECTOR_SIZE; i++)
    {
        sector_buffer[i] = 0xFF;
    }

    stored_tag = (struct st25tb_tag *)(FLASH_STORAGE_ADDRESS + 4);
    new_stored_tag = (struct st25tb_tag *)(sector_buffer + 4);

    count = 0;
    while (stored_tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (index == count)
        {
            found = true;
        }
        else
        {
            memcpy(new_stored_tag, stored_tag, sizeof(struct st25tb_tag));
            new_stored_tag = new_stored_tag + 1;
        }

        stored_tag = stored_tag + 1;
        count++;
    }

    if (!found)
    {
        return -1;
    }

    // Stop other core to avoid losing console uart
    uint core = get_core_num();
    if (core == 1)
    {
        multicore_lockout_start_blocking();
    }

    ints = save_and_disable_interrupts();

    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_STORAGE_OFFSET, sector_buffer, FLASH_SECTOR_SIZE);

    restore_interrupts(ints);

    if (core == 1)
    {
        multicore_lockout_end_blocking();
    }

    return 0;
}

bool st25tb_tag_storage_exists(const uint64_t uid)
{
    struct st25tb_tag *tag = (struct st25tb_tag *)(FLASH_STORAGE_ADDRESS + 4);
    while (tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (tag->uid == uid)
        {
            return true;
        }
        tag = tag + 1;
    }

    return false;
}

uint8_t st25tb_tag_storage_load(const uint64_t uid, struct st25tb_tag *tag)
{
    struct st25tb_tag *stored_tag = (struct st25tb_tag *)(FLASH_STORAGE_ADDRESS + 4);
    while (stored_tag->uid != 0xFFFFFFFFFFFFFFFF)
    {
        if (stored_tag->uid == uid)
        {
            memcpy(tag, stored_tag, sizeof(struct st25tb_tag));
            return 0;
        }
        stored_tag = stored_tag + 1;
    }
    return -1;
}