const METADATA_FIELD_SIZE: usize = 64;
pub const METADATA_TOTAL_SIZE: usize = 5 * METADATA_FIELD_SIZE;
pub const RECORD_SIZE: usize = 16;

#[derive(Debug)]
pub struct MetadataHeader {
    pub title: String,
    pub creator: String,
    pub language: String,
    pub publisher: String,
    pub description: String,
}

impl MetadataHeader {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::with_capacity(METADATA_TOTAL_SIZE);

        let mut write_field = |field: &str| {
            let mut buffer = [0u8; METADATA_FIELD_SIZE];
            let field_bytes = field.as_bytes();
            let len = field_bytes.len().min(METADATA_FIELD_SIZE);
            buffer[..len].copy_from_slice(&field_bytes[..len]);
            bytes.extend_from_slice(&buffer);
        };

        write_field(&self.title);
        write_field(&self.creator);
        write_field(&self.language);
        write_field(&self.publisher);
        write_field(&self.description);

        bytes
    }
}

#[derive(Debug)]
pub struct Record {
    pub data_type: u8,
    pub id: u8,
    pub _reserved_1: [u8; 2],
    pub offset: u32,
    pub size: u32,
    pub _reserved_2: [u8; 4],
}

impl Record {
    pub fn to_bytes(&self) -> [u8; RECORD_SIZE] {
        let mut bytes = [0u8; RECORD_SIZE];

        bytes[0] = self.data_type;
        bytes[1] = self.id;
        bytes[4..8].copy_from_slice(&self.offset.to_be_bytes());
        bytes[8..12].copy_from_slice(&self.size.to_be_bytes());

        bytes
    }
}
