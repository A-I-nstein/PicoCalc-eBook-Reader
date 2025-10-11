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
        char text_buffer[1];
        int text_screen_cur_pos = 0;
        char text_screen[SCREEN_AREA];

        for (int x = 0; x < SCREEN_AREA - 1; x++)
        {
            if (text_screen_cur_pos >= block->size)
            {
                break;
            }
            if (fat32_read(file, text_buffer, 1, &bytes_read) != FAT32_OK)
            {
                printf("Error: Failed to parse block.\n");
                return false;
            }
            else
            {
                text_screen[text_screen_cur_pos] = text_buffer[0];
                text_screen_cur_pos += 1;
                if (text_buffer[0] == '\n')
                {
                    x += SCREEN_SIZE_X - (x % SCREEN_SIZE_X);
                }
                else if (text_buffer[0] == '\t')
                {
                    x += 8;
                }
            }
            current_pos += 1;
        }

        text_screen[text_screen_cur_pos] = '\0';
        printf("\033[2J\033[H");
        printf("%s", text_screen);
        if (!wait_for_enter())
        {
            return false;
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