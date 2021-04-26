
#ifndef __BTAR_H
#define __BTAR_H

#include <stdint.h>

/*
 *        BTAR ARCHIVE FORMAT
 * +-----------------------------------+
 * | +-------------------------------+ |
 * | | Magic Number (8 bytes)        | |
 * | | Creation Date (8 bytes)       | |   One master block
 * | | Number of Files (8 bytes)     | |
 * | | Total Header Length (8 bytes) | |
 * | +-------------------------------+ |
 * |                                   |
 * | +------------------------------+  |
 * | | +--------------------------+ |  |
 * | | | File Length (8 bytes)    | |  |   Many file metadata blocks
 * | | | Archive Offset (8 bytes) | |  |   (one per file in archive)
 * | | +--------------------------+ |  |
 * | |            ...               |  |
 * | |            ...               |  |
 * | |            ...               |  |
 * | +------------------------------+  |
 * |                                   |
 * | +---------------+                 |
 * | | "filename1"\0 |                 |   Null terminated file names
 * | | "filename2"\0 |                 |   (one per file in archive)
 * | |    ...        |                 |
 * | | "filenameN"\0 |                 |
 * | +---------------+                 |
 * |                                   |
 * |           (end of header)         |
 * |                                   |
 * | +-------------------------------+ |
 * | |                               | |  Bytes of files in archive
 * | |         Raw File Data         | |  (one extent per file)
 * | |                               | |  (indexed by metadata blocks)
 * | +-------------------------------+ |
 * +-----------------------------------+
 *
*/


#define BTAR_MAGIC ("\x9a\x00\xA1\xFC""btar")
#define BTAR_MAGIC_SIZE 8


/*
    Describes the entire archive.
*/
struct btar_archive_header {
    /// Used to identify this file as a btar archive
    char magic[BTAR_MAGIC_SIZE];
    /// Seconds since unix epoch this archive was created
    int64_t creation_date;
    /// How many files are in the archive
    uint64_t num_files;
    /// How long is the entire header (including metadata below)
    uint64_t header_length;
};


// TODO: make it intersting and don't use a fixed length filename

/*
    Describes a single file in this archive.
*/
struct btar_file_metadata {
    /// Where in the archive is the first byte of this file
    uint64_t archive_offset;
    /// How many bytes long is this file
    uint64_t num_bytes;
};



#endif /* __BTAR_H */
