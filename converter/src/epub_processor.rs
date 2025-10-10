use crate::file_format::{METADATA_TOTAL_SIZE, MetadataHeader, RECORD_SIZE, Record};
use epub::doc::EpubDoc;
use scraper::{Html, Selector};
use std::{
    error::Error,
    fs::File,
    io::{BufReader, Write},
    path::Path,
};

#[derive(Debug)]
pub struct Book {
    metadata: MetadataHeader,
    header: Vec<Record>,
    body: Vec<String>,
    epub_doc: EpubDoc<BufReader<File>>,
}

fn extract_all_text(html_content: &str) -> String {
    let document = Html::parse_document(html_content);

    let selector = Selector::parse("body").unwrap();

    if let Some(body) = document.select(&selector).next() {
        let all_text = body.text().collect::<Vec<_>>().join(" ");
        let standardized_text = all_text.replace('\u{a0}', " ");
        return standardized_text;
    }

    document
        .root_element()
        .text()
        .collect::<Vec<_>>()
        .join(" ")
        .split_whitespace()
        .collect::<Vec<_>>()
        .join(" ")
}

impl Book {
    pub fn convert(&mut self) -> Result<(), Box<dyn Error>> {
        let output_file_path = self.metadata.title.clone() + ".book";
        let mut output = File::create(&output_file_path)?;

        eprintln!("Started parsing the file.");
        self.parse()?;
        eprintln!("Completed parsing the file.");

        output.write_all(&self.metadata.to_bytes())?;
        eprintln!("Wrote meta-data header to {output_file_path}",);

        for record in &self.header {
            output.write_all(&record.to_bytes())?;
        }
        eprintln!("Wrote header to {output_file_path}",);

        for text in &self.body {
            output.write_all(text.as_bytes())?;
        }
        eprintln!("Wrote content to {output_file_path}",);

        eprintln!(
            "Successfully packed {} pieces of data into {}",
            &self.header.len(),
            output_file_path
        );

        Ok(())
    }

    pub fn new(path_to_file: impl AsRef<Path>) -> Result<Self, Box<dyn Error>> {
        let epub_doc = EpubDoc::new(path_to_file.as_ref())?;

        let get_mdata = |key: &str| -> String { epub_doc.mdata(key).unwrap_or_default() };

        let metadata = MetadataHeader {
            title: get_mdata("title"),
            creator: get_mdata("creator"),
            language: get_mdata("language"),
            publisher: get_mdata("publisher"),
            description: extract_all_text(&get_mdata("description")),
        };

        if metadata.title.is_empty() {
            return Err("Could not find book title (dc:title metadata is missing).")
                .map_err(|e| e.into());
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

            let html_document = Html::parse_document(&raw_page_content);

            let document_text: String = html_document
                .select(&content_selector)
                .filter_map(|element| {
                    let text = element.text().collect::<Vec<_>>().join(" ");
                    let normalized_text: String =
                        text.split_whitespace().collect::<Vec<_>>().join(" ");
                    if normalized_text.is_empty() {
                        None
                    } else {
                        if element.value().name() == "p" {
                            Some(format!("\t{}\n", normalized_text))
                        } else {
                            Some(format!("{}\n", normalized_text))
                        }
                    }
                })
                .collect();

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
}
