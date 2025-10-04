#ifndef BOOK_PARSER_H
#define BOOK_PARSER_H

#include "drivers/fat32.h"

extern const size_t HEADER_BLOCK_SIZE;
extern const size_t METADATA_BLOCK_SIZE;

typedef struct header_block
{
    uint8_t data_type;
    uint8_t section_id;
    uint8_t _reserved_1[2];
    uint32_t offset;
    uint32_t size;
    uint8_t _reserved_2[4];
} header_block;

typedef struct metadata_block
{
    char title[64];
    char creator[64];
    char language[64];
    char publisher[64];
    char description[64];
} metadata_block;

void extract_header_block(header_block *block, const uint8_t *bytes);
void extract_metadata_block(metadata_block *block, const uint8_t *bytes);
bool parse_remaining_headers(fat32_file_t *file, header_block *headers, size_t no_of_header_blocks);

#endif