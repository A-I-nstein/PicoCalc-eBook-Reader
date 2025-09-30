#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "drivers/picocalc.h"
#include "drivers/fat32.h"

typedef struct header_block header_block;

const size_t HEADER_BLOCK_SIZE = 16;
const size_t SCREEN_SIZE_X = 40;
const size_t SCREEN_SIZE_Y = 32;
const size_t SCREEN_AREA = SCREEN_SIZE_X * SCREEN_SIZE_Y;
const bool user_interrupt = false;

struct header_block
{
    uint8_t data_type;
    uint8_t section_id;
    uint8_t _reserved_1[2];
    uint32_t offset;
    uint32_t size;
    uint8_t _reserved_2[4];
};

uint32_t be_bytes_to_uint32(const uint8_t *bytes)
{
    uint32_t result = 0;

    result |= (uint32_t)bytes[0] << 24;
    result |= (uint32_t)bytes[1] << 16;
    result |= (uint32_t)bytes[2] << 8;
    result |= (uint32_t)bytes[3];

    return result;
}

void extract_header_block(header_block *block, const uint8_t *bytes)
{
    block->data_type = bytes[0];
    block->section_id = bytes[1];
    block->offset = be_bytes_to_uint32(&bytes[4]);
    block->size = be_bytes_to_uint32(&bytes[8]);

    // printf("%u %u %u %u\n", block->data_type, block->section_id, block->offset, block->size);
}

int main()
{
    stdio_init_all();
    picocalc_init();
    sleep_ms(1000);

    fat32_file_t file;
    fat32_open(&file, "The House of the Dead.book");

    size_t bytes_read;
    char read_buffer[HEADER_BLOCK_SIZE];
    fat32_error_t result = fat32_read(&file, read_buffer, 16, &bytes_read);

    size_t no_of_header_blocks;

    if (result == FAT32_OK && bytes_read > 0)
    {
        header_block block_1;
        extract_header_block(&block_1, &read_buffer[0]);
        no_of_header_blocks = block_1.offset / HEADER_BLOCK_SIZE;
    }
    // printf("No. of blocks: %u\n", no_of_header_blocks);

    fat32_seek(&file, 0);

    header_block header[no_of_header_blocks];

    for (size_t x = 0; x < no_of_header_blocks; x++)
    {
        header_block block;
        result = fat32_read(&file, read_buffer, 16, &bytes_read);

        if (result == FAT32_OK && bytes_read > 0)
        {
            extract_header_block(&block, &read_buffer[0]);
            header[x] = block;
        }
    }

    for (size_t x = 0; x < no_of_header_blocks; x++)
    {
        if (header[x].size <= SCREEN_AREA)
        {
            fat32_seek(&file, header[x].offset);

            size_t bytes_read;
            char read_buffer[header[x].size + 1];
            fat32_error_t result = fat32_read(&file, read_buffer, header[x].size, &bytes_read);

            if (result == FAT32_OK && bytes_read > 0)
            {
                read_buffer[bytes_read] = '\0';
                printf("\033[2J\033[H");
                for (int i = 0; i < bytes_read; i++)
                {
                    if (read_buffer[i] == '\n')
                    {
                        read_buffer[i] = ' ';
                    }
                }
                printf("%s", read_buffer);
                while (true)
                {
                    char ch = getchar();
                    if (ch == '\r')
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            uint32_t current_char_index = 0;
            uint32_t remaining_chars_len = 0;
            while (current_char_index < header[x].size)
            {
                remaining_chars_len = header[x].size - current_char_index;

                if (remaining_chars_len <= SCREEN_AREA)
                {
                    fat32_seek(&file, header[x].offset + current_char_index);

                    size_t bytes_read;
                    char read_buffer[remaining_chars_len + 1];
                    fat32_error_t result = fat32_read(&file, read_buffer, remaining_chars_len, &bytes_read);

                    if (result == FAT32_OK && bytes_read > 0)
                    {
                        read_buffer[bytes_read] = '\0';
                        printf("\033[2J\033[H");
                        for (int i = 0; i < bytes_read; i++)
                        {
                            if (read_buffer[i] == '\n')
                            {
                                read_buffer[i] = ' ';
                            }
                        }
                        printf("%s", read_buffer);
                        while (true)
                        {
                            char ch = getchar();
                            if (ch == '\r')
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    fat32_seek(&file, header[x].offset + current_char_index);

                    size_t bytes_read;
                    char read_buffer[SCREEN_AREA + 1];
                    fat32_error_t result = fat32_read(&file, read_buffer, SCREEN_AREA, &bytes_read);

                    if (result == FAT32_OK && bytes_read > 0)
                    {
                        read_buffer[bytes_read] = '\0';
                        printf("\033[2J\033[H");
                        for (int i = 0; i < bytes_read; i++)
                        {
                            if (read_buffer[i] == '\n')
                            {
                                read_buffer[i] = ' ';
                            }
                        }
                        printf("%s", read_buffer);
                        while (true)
                        {
                            char ch = getchar();
                            if (ch == '\r')
                            {
                                break;
                            }
                        }
                    }
                }

                current_char_index += SCREEN_AREA;
            }
        }
    }

    fat32_close(&file);

    while (1)
    {
    }
}