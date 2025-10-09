#include "drivers/fat32.h"

#include "book_parser.h"

#define SCREEN_SIZE_X 40
#define SCREEN_SIZE_Y 32
#define SCREEN_AREA (SCREEN_SIZE_X * SCREEN_SIZE_Y)

#define MAX_BOOK_FILES 9
#define MAX_FILENAME_LEN 256

void wait_for_enter();
bool display_section(fat32_file_t *file, const header_block *block);
bool display_metadata(const metadata_block *block);
size_t get_book_files(char file_names[MAX_BOOK_FILES][MAX_FILENAME_LEN]);
int select_book(const char file_names[MAX_BOOK_FILES][MAX_FILENAME_LEN], size_t count);