#include <stdio.h>
#include <string.h>

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

bool display_metadata(const metadata_block *block)
{
    printf("\033[2J\033[H");
    printf("Title      : %s\n", block->title);
    printf("Creator    : %.64s\n", block->creator);
    printf("Language   : %.64s\n", block->language);
    printf("Publisher  : %.64s\n", block->publisher);
    printf("Description: %.64s\n", block->description);
    printf("\nPress Enter to continue...\n");
    wait_for_enter();
    return true;
}

size_t get_book_files(char filenames[][MAX_FILENAME_LEN])
{
    fat32_file_t dir;
    fat32_entry_t dir_entry;
    size_t count = 0;

    if (fat32_open(&dir, ".") != FAT32_OK)
        return 0;

    do
    {
        if (fat32_dir_read(&dir, &dir_entry) != FAT32_OK)
            break;
        if (dir_entry.filename[0] && strstr(dir_entry.filename, ".book") != NULL)
        {
            strncpy(filenames[count], dir_entry.filename, MAX_FILENAME_LEN - 1);
            filenames[count][MAX_FILENAME_LEN - 1] = '\0';
            count++;
            if (count >= MAX_BOOK_FILES)
                break;
        }
    } while (dir_entry.filename[0]);

    fat32_close(&dir);
    return count;
}

int select_book(const char filenames[][MAX_FILENAME_LEN], size_t count)
{
    printf("\033[2J\033[H");
    printf("Available .book files:\n");
    for (size_t i = 0; i < count; i++)
    {
        printf("%zu: %s\n", i + 1, filenames[i]);
    }
    printf("Select a book by number: ");

    int choice = 0;
    int ch = 0;
    while (1)
    {
        ch = getchar();
        printf("%c\n", ch);
        if (ch >= '1' && ch <= '9')
        {
            choice = ch - '0';
            if (choice >= 1 && (size_t)choice <= count)
                printf("\nPress Enter to continue...\n");
                break;
        }
        printf("\nInvalid selection. Try again: ");
    }
    while (ch != '\n' && ch != '\r') ch = getchar();
    return choice - 1;
}