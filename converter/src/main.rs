mod epub_processor;
mod file_format;

use epub_processor::Book;
use std::io::{Write, stdin, stdout};

fn main() {
    let result = run();
    if let Err(e) = result {
        eprintln!("Error: {e}");
    }
    println!("\n\nPress Enter to exit...");
    let mut exit_wait = String::new();
    let _ = stdin().read_line(&mut exit_wait);
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let mut input_path = String::new();
    let mut output_path = String::new();

    print!("***************************************\n");
    print!("* Welcome to PicoCalc eBook Converter *\n");
    print!("*       Developed by A-I-nstein       *\n");
    print!("***************************************\n");

    print!("\nEnter path to input .epub file: ");
    stdout().flush()?;
    stdin().read_line(&mut input_path)?;
    let input_path = input_path.trim().trim_matches('"');

    print!("Enter path to output .book file: ");
    stdout().flush()?;
    stdin().read_line(&mut output_path)?;
    let output_path = output_path.trim();

    let mut book = Book::new(input_path)?;
    book.convert_to(output_path)?;

    Ok(())
}
