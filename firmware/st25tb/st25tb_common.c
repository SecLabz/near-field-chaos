#include "st25tb_common.h"


uint8_t st25tb_tag_parse_from_string(const char *str, struct st25tb_tag *tag)
{
    int8_t result, i;
    // 152 is the number of single char in a tag
    if (strlen(str) != ST25TB_TAG_BYTES_SIZE * 2)
    {
        return -1;
    }

    for (i = 0; i < ST25TB_TAG_BYTES_SIZE / 4 - 2; i++)
    {
        result = sscanf(str + (i * 8), "%8x", &tag->blocks[i]);
        if (result != 1)
        {
            return -1;
        }
    }

    // ignore otp

    result = sscanf(str + ST25TB_TAG_BYTES_SIZE * 2 - 16, "%llx", &tag->uid);
    if (result != 1)
    {
        return -1;
    }
    return 0;
}