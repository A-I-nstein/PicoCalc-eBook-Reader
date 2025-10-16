# PicoCalc eBook Reader

## Overview

**PicoCalc eBook Reader** is a multi-component project for converting, storing, and reading eBooks in a custom `.book` format, optimized for embedded devices—specifically the **PicoCalc** from [Clockwork Pi](https://clockworkpi.com/). The project is written in Rust (converter) and C (firmware/reader).

This project is built on top of [BlairLeduc/picocalc-text-starter](https://github.com/BlairLeduc/picocalc-text-starter/tree/main), which serves as the base driver for hardware support (./firmware/drivers).

## Components

- **converter** (Rust): Converts EPUB files to the `.book` format, extracting and embedding metadata.
- **firmware** (C): Embedded firmware for reading `.book` files on PicoCalc hardware, with display, keyboard, and SD card support.

## Features

- Extracts and cleans readable text from EPUB files.
- Packs all text into a single `.book` file with a fast-access header for efficient navigation.
- Extracts and embeds book metadata (title, author, description, etc.) for use by the reader.
- Utilizes book metadata for display and navigation.
- Lists all books and allows user selection.
- Startup screen and improved text formatting (paragraphs, tabs, pagination).
- Section/chapter navigation.
- Modular, maintainable codebase.

## Navigation and Usage

- On startup, the firmware lists all `.book` files on the SD card (only the first 9 books for now) and allows the user to select a book.
- Book metadata (title, author, etc.) is displayed after selecting a book.
- Use the PicoCalc keyboard to navigate between pages and chapters.
    - Press `Enter` to go to the next page.
    - Press `<` to go to the previous section/chapter.
    - Press `>` to go to the next section/chapter.
    - Press the `q` key to return to the main book list at any time.

## Planned / In Progress

- Word breaks (next line/page), font size/style options.
- Table of Contents navigation.
- Support for ASCII control characters and new features.
- Display cover pages and images.
- Show battery and pages remaining, time to complete section/book.
- Bookmarks.
- Increase book limits

## .book File Format

The `.book` file has three main parts:

1. **Metadata**: Contains book information such as title, author, description, and other relevant fields extracted from the EPUB. 
2. **Header**: Sequence of 16-byte records, one per section, for fast lookup.
3. **Body**: Concatenated text content of all sections.

### Metadata Structure

The metadata section at the start of each `.book` file contains key information about the book.  
Each field is stored as a fixed-length UTF-8 byte array.

| Field       | Type   | Length (bytes) | Description         |
|-------------|--------|----------------|---------------------|
| Title       | String | 64             | Book title          |
| Author      | String | 64             | Author/creator      |
| Language    | String | 64             | Language code       |
| Publisher   | String | 64             | Publisher name      |
| Description | String | 64             | Book description    |

- Each field is padded or truncated to exactly 64 bytes.
- All fields are concatenated in the above order at the start of the `.book` file.

### Header Record Structure

| Byte  | Data Type | Description                                                                    |
|-------|-----------|--------------------------------------------------------------------------------|
| 0     | u8        | Data Type (e.g., 0x00 for body text)                                           |
| 1     | u8        | Section ID                                                                     |
| 2-3   | [u8; 2]   | Reserved (unused)                                                              |
| 4-7   | u32       | Offset: Start byte of this section's text in the body                          |
| 8-11  | u32       | Size: Length in bytes of this section's text                                   |
| 12-15 | [u8; 4]   | Reserved (unused)                                                              |

## Usage

### Releases

Pre-built release versions of the converter and firmware will be made available on the GitHub Releases page. You can download the latest binaries and firmware images from there for easier setup and deployment.

### Build

#### Convert an EPUB to .book

```sh
cd converter
cargo run --release
```

#### Flash and Run Firmware

1. Build the firmware using CMake for the Raspberry Pi Pico or compatible device.
2. Flash the generated `.uf2` file to your device.
3. Place `.book` files on the SD card and insert into the device.

## Project Status

- **converter**: Stable and functional, including metadata extraction.
- **firmware/reader**: Core features implemented; advanced features in progress.

## License

MIT License.

---

**Hardware Target:** PicoCalc by Clockwork Pi  
**Base Driver:** [BlairLeduc/picocalc-text-starter](https://github.com/BlairLeduc/picocalc-text-starter/tree/main)