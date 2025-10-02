use std::{
    error::Error,
    fs::File,
    io::{BufReader, Write},
};

use epub::doc::EpubDoc;
use scraper::{Html, Selector};

const RECORD_SIZE: usize = 16;
const METADATA_FIELD_SIZE: usize = 64;
const METADATA_TOTAL_SIZE: usize = 5 * METADATA_FIELD_SIZE;

#[derive(Debug)]
struct MetadataHeader {
    title: String,
    creator: String,
    language: String,
    publisher: String,
    description: String,
}

impl MetadataHeader {
    fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::with_capacity(METADATA_TOTAL_SIZE);

        let mut string_to_fixed_bytes = |s: &String| {
            let s_bytes = s.as_bytes();
            let len = s_bytes.len().min(METADATA_FIELD_SIZE);
            bytes.extend_from_slice(&s_bytes[..len]);

            if len < METADATA_FIELD_SIZE {
                bytes.extend(vec![0; METADATA_FIELD_SIZE - len]);
            }
        };

        string_to_fixed_bytes(&self.title);
        string_to_fixed_bytes(&self.creator);
        string_to_fixed_bytes(&self.language);
        string_to_fixed_bytes(&self.publisher);
        string_to_fixed_bytes(&self.description);

        bytes
    }
}

#[derive(Debug)]
struct Record {
    data_type: u8,
    id: u8,
    _reserved_1: [u8; 2],
    offset: u32,
    size: u32,
    _reserved_2: [u8; 4],
}

impl Record {
    fn to_bytes(&self) -> [u8; RECORD_SIZE] {
        let mut bytes = [0u8; RECORD_SIZE];

        bytes[0] = self.data_type;
        bytes[1] = self.id;
        bytes[4..8].copy_from_slice(&self.offset.to_be_bytes());
        bytes[8..12].copy_from_slice(&self.size.to_be_bytes());

        bytes
    }
}

#[derive(Debug)]
pub struct Book {
    metadata: MetadataHeader,
    header: Vec<Record>,
    body: Vec<String>,
    epub_doc: EpubDoc<BufReader<File>>,
}

impl Book {
    pub fn new(path_to_file: &str) -> Result<Self, Box<dyn Error>> {
        let epub_doc = EpubDoc::new(path_to_file)?;

        let get_mdata = |key: &str| -> String { epub_doc.mdata(key).unwrap_or_default() };

        let metadata = MetadataHeader {
            title: get_mdata("title"),
            creator: get_mdata("creator"),
            language: get_mdata("language"),
            publisher: get_mdata("publisher"),
            description: get_mdata("description"),
        };

        if metadata.title.is_empty() {
            return Err(Box::from(
                "Could not find book title (dc:title metadata is missing).",
            ));
        }

        Ok(Self {
            metadata,
            header: Vec::new(),
            body: Vec::new(),
            epub_doc,
        })
    }

    fn parse(&mut self) -> Result<(), Box<dyn Error>> {
        let num_pages = self.epub_doc.get_num_pages();
        let mut current_offset = (num_pages * RECORD_SIZE) as u32 + METADATA_TOTAL_SIZE as u32;
        let content_selector = Selector::parse("p, h1, h2, h3, h4, h5, h6, li")?;

        for id in 0..num_pages {
            let (raw_page_content, _page_type) = self
                .epub_doc
                .get_current_str()
                .ok_or("Could not process file.")?;

            let mut document_text = String::new();
            let html_document = Html::parse_document(&raw_page_content);

            for element in html_document.select(&content_selector) {
                let element_text = element.text().collect::<Vec<_>>().join(" ");
                if !element_text.trim().is_empty() {
                    document_text.push_str(&element_text);
                    document_text.push('\n');
                }
            }

            let text_size = document_text.len() as u32;
            self.header.push(Record {
                data_type: 0x00,
                id: id as u8,
                _reserved_1: [0; 2],
                offset: current_offset,
                size: text_size,
                _reserved_2: [0; 4],
            });
            self.body.push(document_text);

            current_offset += text_size;
            self.epub_doc.go_next();
        }
        Ok(())
    }

    pub fn convert(&mut self) -> Result<(), Box<dyn Error>> {
        let output_file_path = self.metadata.title.clone() + ".book";
        let mut output = File::create(&output_file_path)?;

        println!("Started parsing the file.");
        self.parse()?;
        println!("Completed parsing the file.");

        output.write_all(&self.metadata.to_bytes())?;
        println!("Wrote meta-data header to {output_file_path}",);

        for record in &self.header {
            output.write_all(&record.to_bytes())?;
        }
        println!("Wrote header to {output_file_path}",);

        for text in &self.body {
            output.write_all(text.as_bytes())?;
        }
        println!("Wrote content to {output_file_path}",);

        println!(
            "Successfully packed {} pieces of data into {}",
            &self.header.len(),
            output_file_path
        );

        Ok(())
    }
}

fn main() -> Result<(), Box<dyn Error>> {
    let mut book =
        Book::new("../books/fyodor-dostoevsky_the-house-of-the-dead_constance-garnett.epub")?;
    book.convert()?;

    Ok(())
}
