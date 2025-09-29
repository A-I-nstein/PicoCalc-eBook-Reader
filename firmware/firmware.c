#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "drivers/fat32.h"

static void get_str_size(char *buffer, uint32_t buffer_size, uint64_t bytes)
{
    const char *unit = "bytes";
    uint32_t divisor = 1;

    if (bytes >= 1000 * 1000 * 1000)
    {
        divisor = 1000 * 1000 * 1000;
        unit = "GB";
    }
    else if (bytes >= 1000 * 1000)
    {
        divisor = 1000 * 1000;
        unit = "MB";
    }
    else if (bytes >= 1000)
    {
        divisor = 1000;
        unit = "KB";
    }

    if (strcmp(unit, "bytes") == 0 || strcmp(unit, "KB") == 0)
    {
        snprintf(buffer, buffer_size, "%llu %s", (unsigned long long)(bytes / divisor), unit);
    }
    else
    {
        snprintf(buffer, buffer_size, "%.1f %s", ((float)bytes) / divisor, unit);
    }
}

void sd_dir_dirname(const char *dirname)
{
    fat32_file_t dir;
    fat32_entry_t dir_entry;

    fat32_error_t result = fat32_open(&dir, dirname);
    if (result != FAT32_OK)
    {
        printf("Error: %s\n", fat32_error_string(result));
        return;
    }

    do
    {
        result = fat32_dir_read(&dir, &dir_entry);
        if (result != FAT32_OK)
        {
            printf("Error: %s\n", fat32_error_string(result));
            return;
        }
        if (dir_entry.filename[0])
        {
            if (dir_entry.attr & (FAT32_ATTR_VOLUME_ID | FAT32_ATTR_HIDDEN | FAT32_ATTR_SYSTEM))
            {
                // It's a volume label, hidden file, or system file, skip it
                continue;
            }
            else if (dir_entry.attr & FAT32_ATTR_DIRECTORY)
            {
                // It's a directory, append '/' to the name
                printf("%s/\n", dir_entry.filename);
            }
            else
            {
                char size_buffer[16];
                get_str_size(size_buffer, sizeof(size_buffer), dir_entry.size);
                printf("%-28s %10s\n", dir_entry.filename, size_buffer);
            }
        }
    } while (dir_entry.filename[0]);

    fat32_close(&dir);
}

void dir()
{
    sd_dir_dirname("."); // Show root directory
}

uint32_t be_bytes_to_uint32(const uint8_t *bytes) {
    uint32_t result = 0;
    
    result |= (uint32_t)bytes[0] << 24;
    result |= (uint32_t)bytes[1] << 16;
    result |= (uint32_t)bytes[2] << 8;
    result |= (uint32_t)bytes[3];
    
    return result;
}

int main()
{
    stdio_init_all();

    sleep_ms(5000);

    printf("Initialising...\n");
    fat32_init();
    printf("Initialisation completed.\n");

    sleep_ms(5000);

    dir();

    fat32_file_t file;

    uint32_t current_position = 0;

    fat32_open(&file, "The House of the Dead.book");

    char read_buffer[16];
    size_t bytes_read;

    fat32_error_t result = fat32_read(&file, read_buffer, 16, &bytes_read);

    printf("%zu bytes read \n", bytes_read);

    if (result == FAT32_OK && bytes_read > 0)
    {
        for (size_t i = 0; i < bytes_read; i++)
        {
            printf("%02X ", (unsigned char)read_buffer[i]);
        }
    }
    printf("\n");

    uint32_t section_start_pos = be_bytes_to_uint32(&read_buffer[4]);
    uint32_t section_size = be_bytes_to_uint32(&read_buffer[8]);

    printf("Section starts at: %u\n", section_start_pos);
    printf("Section size: %u\n", section_size);

    char read_buffer_2[section_size];
    size_t bytes_read_2;

    fat32_seek(&file, section_start_pos);
    fat32_error_t result_2 = fat32_read(&file, read_buffer_2, section_size, &bytes_read_2);

    printf("Section: %s", read_buffer_2);

    fat32_close(&file);

    while (1)
    {
    }
}