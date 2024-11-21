#include "console.h"

char command[CONSOLE_LINE_BUFFER_LENGTH];

int console_parse_command(char *argv[], int max_size)
{
    char *token;
    uint8_t argc = 0;

    token = strtok(command, " ");
    while (token != NULL)
    {
        argv[argc] = strdup(token);
        argc++;
        if (argc > max_size)
        {
            break;
        }
        token = strtok(NULL, " ");
    }

    return argc;
}

int argc;
char *argv[10];
void (*handle_fn)(const int argc, char *argv[]);

void console_command_task()
{
    int i;
    printf("\n");
    handle_fn(argc, argv);
    for (i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    printf("\nnfc> ");
}

void console_handle_command()
{
    int i;
    bool found = false;
    char *str;

    if (strlen(command) == 0)
    {
        printf("\nnfc> ");
        return;
    }

    argc = console_parse_command(argv, sizeof(argv) / sizeof(argv[0]));
    if (argc == 0)
    {
        printf("\nnfc> ");
        return;
    }

    for (i = 0; i < sizeof(console_commands) / sizeof(console_commands[0]); i++)
    {
        if (strcmp(console_commands[i].name, argv[0]) == 0)
        {
            found = true;
            handle_fn = console_commands[i].handle_fn;
            task_start(console_command_task);
            break;
        }
    }

    if (!found)
    {
        printf("\nUnknown command \"%s\".\n\n", command);
        console_print_help(argc, argv);
        printf("\nnfc> ");
    }
}

void console_start()
{
    int16_t c;
    char input_buffer[CONSOLE_LINE_BUFFER_LENGTH];
    size_t cursor = 0;
    bool task_done = false;

    memset(input_buffer, '\0', CONSOLE_LINE_BUFFER_LENGTH);
    memset(command, '\0', CONSOLE_LINE_BUFFER_LENGTH);
    console_flush_input();

    while (true)
    {
        c = getchar();

        if (task_is_running())
        {
            // CTRL+C or q
            if (c == 0x3 || c == 'q')
            {
                task_stop();
            }
            else
            {
                continue;
            }
        }
        else
        {
            if (c == 0x3 || c == 'q')
            {
                printf("\nnfc> ");
                continue;
            }
        }

        // skip specials characters
        if (c <= 0x07 ||
            c >= 0x09 && c <= 0x0c ||
            c >= 0x0e && c <= 0x1a ||
            c >= 0x1c && c <= 0x1f)
        {
            continue;
        }

        // escape
        if (c == 0x1b)
        {
            console_flush_input();
            continue;
        }

        // handle enter
        if (c == 0x0d)
        {
            memcpy(command, input_buffer, CONSOLE_LINE_LENGTH);
            memset(input_buffer, '\0', CONSOLE_LINE_BUFFER_LENGTH);
            cursor = 0;
            console_handle_command();
            continue;
        }

        // handle backspace & del
        if (c == 0x7f || c == 0x08)
        {
            if (cursor == 0)
            {
                continue;
            }
            cursor = console_delete_char(input_buffer);

            // use backspace to erase character on terminal
            printf("%c%c%c", 0x08, ' ', 0x08);
            continue;
        }
        // avoid overflow
        if (cursor == CONSOLE_LINE_LENGTH)
            continue;

        // handle character input
        cursor = console_append_char(input_buffer, c);
        printf("%c", c);
    }
}

size_t console_clear_buffer(char *buffer)
{
    memset(buffer, '\0', CONSOLE_LINE_BUFFER_LENGTH);
    return strlen(buffer);
}

size_t console_append_char(char *buffer, char c)
{
    if (strlen(buffer) == CONSOLE_LINE_LENGTH)
    {
        return CONSOLE_LINE_LENGTH;
    }
    buffer[strlen(buffer)] = c;
    return strlen(buffer);
}

size_t console_delete_char(char *buffer)
{
    if (strlen(buffer) == 0)
        return 0;
    buffer[strlen(buffer) - 1] = '\0';
    return strlen(buffer);
}

void console_flush_input()
{
    int16_t c;
    do
    {
        c = getchar_timeout_us(500);
    } while (c != PICO_ERROR_TIMEOUT && c != PICO_ERROR_GENERIC);
}