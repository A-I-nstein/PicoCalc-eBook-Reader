# eBook Reader

## EPUB to .book Converter (converter)

This is a command-line tool written in Rust that converts EPUB files into a custom binary format with a .book extension. The primary purpose is to parse the textual content of an EPUB, extract only the readable text from each section, and pack it into a single, structured file.

The output .book file is designed for efficient reading, with a header that allows for quick navigation to any section of the book.

### The .book File Format

The output .book file consists of two main parts:

    Header: A series of 16-byte records. Each record corresponds to a section of the original EPUB and contains metadata about the text content.

    Body: The raw, concatenated text content of the entire book, following the header.

### Header Record Structure

Each 16-byte record in the header follows this structure:

| Byte  | Data Type | Description                                                                    |
|-------|-----------|--------------------------------------------------------------------------------|
| 0     | u8        | Data Type (e.g., 0x00 for body text)                                           |
| 1     | u8        | Section ID                                                                     |
| 2-3   | [u8; 2]   | Reserved (unused)                                                              |
| 4-7   | u32       | Offset: The starting byte position of this section's text in  the file's body. |
| 8-11  | u32       | Size: The size in bytes of this section's text.                                |
| 12-15 | [u8; 4]   | Reserved (unused)                                                              |

