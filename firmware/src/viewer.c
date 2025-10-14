#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include "include/viewer.h"

bool wait_for_enter()
{
    while (true)
    {
        int ch = getchar();
        if (ch == '\r')
        {
            return true;
        }
        else if (ch == 'q')
        {
            return false;
        }
    }
}

bool display_file(fat32_file_t *file, const uint32_t start_pos, const uint32_t end_pos)
{
    uint32_t current_file_pos = start_pos;

    while (current_file_pos < end_pos)
    {
        if (fat32_seek(file, current_file_pos) != FAT32_OK)
        {
            printf("Error: Unable to seek file location: %u\n", current_file_pos);
            return false;
        }

        char buffer_text[SCREEN_AREA + 1];
        size_t bytes_to_read = SCREEN_AREA;
        size_t bytes_read;

        if ((current_file_pos + SCREEN_AREA) > end_pos)
        {
            bytes_to_read = end_pos - current_file_pos;
        }

        if (fat32_read(file, buffer_text, bytes_to_read, &bytes_read) != FAT32_OK)
        {
            printf("Error: Failed to read text at %u location.\n", current_file_pos);
            return false;
        }

        if (bytes_read == 0)
        {
            break;
        }

        char screen_text[SCREEN_AREA + 1];
        memset(screen_text, 0, sizeof(screen_text));

        size_t screen_char_count = 0;
        size_t screen_idx = 0;
        size_t file_bytes_processed = 0;

        for (int i = 0; i < bytes_read; i++)
        {
            file_bytes_processed++;
            char current_char = buffer_text[i];
            size_t projected_count = screen_char_count;

            if (current_char == '\f')
            {
                break;
            }
            else if (current_char == '\n')
            {
                projected_count += SCREEN_SIZE_X - (screen_char_count % SCREEN_SIZE_X);
            }
            else if (current_char == '\t')
            {
                projected_count += 8 - (screen_char_count % 8);
            }
            else
            {
                projected_count++;
            }

            if (projected_count > SCREEN_AREA)
            {
                file_bytes_processed--;
                break;
            }

            screen_text[screen_idx++] = current_char;
            screen_char_count = projected_count;
        }
        screen_text[screen_idx] = '\0';

        printf("\033[2J\033[H");
        printf("%s", screen_text);

        current_file_pos += file_bytes_processed;

        if (current_file_pos < end_pos)
        {
            if (!wait_for_enter())
            {
                return false;
            }
        }
    }
    return true;
}

bool display_metadata(const metadata_block *block)
{
    printf("\033[2J\033[H");
    printf("About Book:\n\n");
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
    printf("\nAvailable .book files:\n");
    for (size_t i = 0; i < count; i++)
    {
        printf("%zu: %s\n", i + 1, filenames[i]);
    }

    int choice = -1;
    int ch;

    while (choice == -1)
    {
        printf("\nSelect a book by number: ");
        ch = getchar();
        printf("%c\n", ch);

        if (ch >= '1' && ch <= '9')
        {
            int temp_choice = ch - '0';
            if ((size_t)temp_choice >= 1 && (size_t)temp_choice <= count)
            {
                choice = temp_choice;
            }
        }

        if (choice == -1)
        {
            printf("\nInvalid selection. Try again.\n");
        }
    }

    return choice - 1;
}