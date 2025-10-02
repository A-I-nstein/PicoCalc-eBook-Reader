mod file_format;
mod epub_processor;

use epub_processor::Book;
use std::error::Error;

fn main() -> Result<(), Box<dyn Error>> {
    let mut book =
        Book::new("../books/fyodor-dostoevsky_the-house-of-the-dead_constance-garnett.epub")?;
    book.convert()?;

    Ok(())
}
