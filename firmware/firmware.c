#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "drivers/fat32.h"

typedef struct header_block header_block;

const size_t HEADER_BLOCK_SIZE = 16;

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

    printf("%u %u %u %u\n", block->data_type, block->section_id, block->offset, block->size);
}

int main()
{
    stdio_init_all();
    sleep_ms(5000);

    printf("Initialising FAT32\n");
    fat32_init();
    printf("Initialisation completed\n");

    fat32_file_t file;
    uint32_t current_position = 0;
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
    printf("No. of blocks: %u\n", no_of_header_blocks);

    header_block header[no_of_header_blocks];

    for (size_t x = 0; x < no_of_header_blocks; x++)
    {
    }

    // uint32_t section_start_pos = be_bytes_to_uint32(&read_buffer[4]);
    // uint32_t section_size = be_bytes_to_uint32(&read_buffer[8]);

    // printf("Section starts at: %u\n", section_start_pos);
    // printf("Section size: %u\n", section_size);

    // char read_buffer_2[section_size];
    // size_t bytes_read_2;

    // fat32_seek(&file, section_start_pos);
    // fat32_error_t result_2 = fat32_read(&file, read_buffer_2, section_size, &bytes_read_2);

    // printf("Section: %s", read_buffer_2);

    fat32_close(&file);

    while (1)
    {
    }
}