#include <stdio.h>
#include <pico/stdlib.h>

#include "include/book_parser.h"

const size_t HEADER_BLOCK_SIZE = 16;

static uint32_t be_bytes_to_uint32(const uint8_t *bytes)
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
}

bool parse_remaining_headers(fat32_file_t *file, header_block *headers, size_t no_of_header_blocks)
{

    for (size_t x = 0; x < no_of_header_blocks; x++)
    {
        size_t bytes_read;
        char read_buffer[HEADER_BLOCK_SIZE];

        if (fat32_read(file, read_buffer, HEADER_BLOCK_SIZE, &bytes_read) != FAT32_OK)
        {
            printf("Error: Failed to parse book headers.\n");
            return false;
        }
        else
        {
            extract_header_block(&headers[x], (uint8_t *)read_buffer);
        }
    }
    return true;
}
