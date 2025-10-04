#include "drivers/fat32.h"

#include "book_parser.h"

#define SCREEN_SIZE_X 40
#define SCREEN_SIZE_Y 32
#define SCREEN_AREA (SCREEN_SIZE_X * SCREEN_SIZE_Y)

bool display_section(fat32_file_t *file, const header_block *block);
bool display_metadata(const metadata_block *block);