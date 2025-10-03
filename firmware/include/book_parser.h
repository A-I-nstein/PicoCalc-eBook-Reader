#ifndef BOOK_PARSER_H
#define BOOK_PARSER_H

#include "drivers/fat32.h"

extern const size_t HEADER_BLOCK_SIZE;

typedef struct header_block
{
    uint8_t data_type;
    uint8_t section_id;
    uint8_t _reserved_1[2];
    uint32_t offset;
    uint32_t size;
    uint8_t _reserved_2[4];
} header_block;

void extract_header_block(header_block *block, const uint8_t *bytes);
bool parse_remaining_headers(fat32_file_t *file, header_block *headers, size_t no_of_header_blocks);

#endif