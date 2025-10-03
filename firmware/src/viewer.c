#include <stdio.h>

#include <pico/stdlib.h>

#include "include/viewer.h"

static char text_buffer[SCREEN_AREA + 1];

static void wait_for_enter()
{
    while (getchar() != '\r')
    {
        tight_loop_contents();
    }
}

static void prepare_buffer_for_display(size_t bytes_read)
{
    text_buffer[bytes_read] = '\0';
    for (size_t i = 0; i < bytes_read; i++)
    {
        if (text_buffer[i] == '\n')
        {
            text_buffer[i] = ' ';
        }
    }
}

bool display_section(fat32_file_t *file, const header_block *block)
{
    size_t bytes_read;
    uint32_t current_pos = 0;

    if (fat32_seek(file, block->offset) != FAT32_OK)
    {
        printf("Error: Unable to seek file location");
        return false;
    }

    while (current_pos < block->size)
    {
        uint32_t bytes_to_read = block->size - current_pos;

        if (bytes_to_read > SCREEN_AREA)
        {
            bytes_to_read = SCREEN_AREA;
        }

        if (fat32_read(file, text_buffer, bytes_to_read, &bytes_read) != FAT32_OK)
        {
            printf("Error: Failed to parse block.\n");
            return false;
        }
        else
        {
            prepare_buffer_for_display(bytes_read);
            printf("\033[2J\033[H");
            printf("%s", text_buffer);
            wait_for_enter();
        }
        current_pos += bytes_to_read;
    }
    return true;
}