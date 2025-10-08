#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>

#include "drivers/fat32.h"
#include "drivers/picocalc.h"

#include "include/book_parser.h"
#include "include/viewer.h"

const bool user_interrupt = false;

int main()
{
    stdio_init_all();
    picocalc_init();
    sleep_ms(1000);

    fat32_file_t file;
    header_block *headers = NULL;
    metadata_block metadata;

    char book_filenames[MAX_BOOK_FILES][MAX_FILENAME_LEN];
    size_t book_count = get_book_files(book_filenames);

    if (book_count == 0)
    {
        printf("Error: No .book files found.\n");
        return -1;
    }

    int selected = select_book(book_filenames, book_count);

    if (fat32_open(&file, book_filenames[selected]) != FAT32_OK)
    {
        printf("Error: Failed to the open book file.\n");
        return -1;
    }

    size_t bytes_read;
    char metadata_buffer[METADATA_BLOCK_SIZE];
    char read_buffer[HEADER_BLOCK_SIZE];

    if (fat32_read(&file, metadata_buffer, METADATA_BLOCK_SIZE, &bytes_read) != FAT32_OK)
    {
        printf("Error: Failed to parse book metadata.\n");
        goto cleanup;
    }
    else
    {
        extract_metadata_block(&metadata, &metadata_buffer[0]);
        display_metadata(&metadata);
    }

    if (fat32_read(&file, read_buffer, HEADER_BLOCK_SIZE, &bytes_read) != FAT32_OK)
    {
        printf("Error: Failed to parse book headers.\n");
        goto cleanup;
    }
    else
    {
        header_block block_1;
        extract_header_block(&block_1, &read_buffer[0]);

        size_t no_of_header_blocks = block_1.offset / HEADER_BLOCK_SIZE;
        headers = malloc(no_of_header_blocks * sizeof(header_block));

        if (headers == NULL)
        {
            printf("Error: Failed to allocate memory for headers.\n");
            goto cleanup;
        }

        headers[0] = block_1;

        if (!parse_remaining_headers(&file, &headers[1], no_of_header_blocks - 1))
        {
            printf("Error: Failed to parse book headers.\n");
            goto cleanup;
        }
        for (size_t x = 0; x < no_of_header_blocks; x++)
        {
            if (!display_section(&file, &headers[x]))
            {
                printf("Error: Failed to display section %zu.\n", x);
                break;
            }
        }
    }

cleanup:
    if (headers != NULL)
    {
        free(headers);
    }
    fat32_close(&file);
    printf("Book closed.\n");

    while (true)
    {
        tight_loop_contents();
    }
    return 0;
}