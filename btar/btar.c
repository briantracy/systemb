
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "btar.h"


/// btar only accepts files in the current directory
/// This could be easily changed, but making subdirectories
/// when unpacking is a pain.
static int is_valid_filename(const char *const name) {
    return strchr(name, '/') == NULL;
}

static size_t total_filename_bytes(const char *const files[], size_t const numfiles) {
    size_t total = 0;
    for (size_t i = 0; i < numfiles; i++) {
        // include byte for null terminator
        total += strlen(files[i]) + 1;
    }
    return total;
}

static ssize_t include_file(int const archive_fd, int const file_fd) {
    #define BUFF_SIZE 4096
    char buff[BUFF_SIZE];
    ssize_t bytes_read = 0;
    while (1) {
        ssize_t const nr = read(file_fd, buff, BUFF_SIZE);
        if (nr == 0) {
            break;
        }
        if (nr == -1) {
            return -1;
        }
        bytes_read += nr;
        ssize_t const nw = write(archive_fd, buff, (size_t)nr);
        if (nw == 0) {
            break;
        }
        if (nw == -1) {
            return -1;
        }
    }
    return bytes_read;
}

/// Create an archive
int pack(const char *const archive_name, const char *const files[], size_t const numfiles) {
    printf("info: creating archive `%s` from `%zu` files\n", archive_name, numfiles);

    int status = 0;

    // File descriptors to read from
    int *const fds = calloc(numfiles, sizeof(int));
    struct btar_file_metadata *const metadata = calloc(numfiles, sizeof(struct btar_file_metadata));
    if (fds == NULL || metadata == NULL) {
        fprintf(stderr, "error: could not allocate space\n");
        status = 1;
        goto cleanup;
    }

    for (size_t i = 0; i < numfiles; i++) {
        printf("info: opening file `%s`\n", files[i]);
        if (!is_valid_filename(files[i])) {
            fprintf(stderr, "error: file name `%s` not in current directory\n", files[i]);
            status = 1;
            goto cleanup;
        }
        fds[i] = open(files[i], O_RDONLY);
        if (fds[i] == -1) {
            fprintf(stderr, "error: could not open file `%s` for reading\n", files[i]);
            status = 1;
            goto cleanup;
        }
    }

    int const archive_fd = open(archive_name, O_WRONLY | O_CREAT | O_EXCL, 0444);
    if (archive_fd == -1) {
        fprintf(stderr, "error: could not create archive file or archive already exists\n");
        status = 1;
        goto cleanup;
    }

    size_t const header_size =
        sizeof(struct btar_archive_header) +
        (sizeof(struct btar_file_metadata) * numfiles) +
        total_filename_bytes(files, numfiles);

    printf("info: btar header will occupy first %zu bytes of archive\n", header_size);
    printf("info: - archive header: %zu bytes\n", sizeof(struct btar_archive_header));
    printf("info: - file metadata: %zu bytes (%zu bytes * %zu files)\n",
            sizeof(struct btar_file_metadata) * numfiles, sizeof(struct btar_file_metadata), numfiles);
    printf("info: - filenames: %zu bytes\n", total_filename_bytes(files, numfiles));


    if (lseek(archive_fd, (off_t)header_size, SEEK_SET) == -1) {
        fprintf(stderr, "error: could not seek to data section of archive\n");
        status = 1;
        goto cleanup;
    }

    size_t archive_offset = 0;
    /// Include the actual file data in the archive
    for (size_t i = 0; i < numfiles; i++) {
        printf("info: transferring file `%s` to `%s`\n", files[i], archive_name);
        metadata[i].archive_offset = archive_offset;
        ssize_t const file_length = include_file(archive_fd, fds[i]);
        if (file_length == -1) {
            fprintf(stderr, "error: could not read from file `%s`\n", files[i]);
            status = 1;
            goto cleanup;
        }
        metadata[i].num_bytes = (size_t)file_length;
        archive_offset += (size_t)file_length;
    }

    /// Write out the archive header and metadata
    struct btar_archive_header header;
    memcpy((void *)&(header.magic), BTAR_MAGIC, BTAR_MAGIC_SIZE);
    header.creation_date = (int64_t)time(NULL);
    header.num_files = (uint64_t)numfiles;
    header.header_length = header_size;

    if (lseek(archive_fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "error: could not seek in archive\n");
        status = 1;
        goto cleanup;
    }

    if (write(archive_fd, &header, sizeof(struct btar_archive_header)) == -1) {
        fprintf(stderr, "error: could not write archive header\n");
        status = 1;
        goto cleanup;
    }

    for (size_t i = 0; i < numfiles; i++) {
        if (write(archive_fd, &(metadata[i]), sizeof(struct btar_file_metadata)) == -1) {
            fprintf(stderr, "error: could not write file metadata to archive\n");
            status = 1;
            goto cleanup;
        }
    }

    for (size_t i = 0; i < numfiles; i++) {
        // include null terminator in name
        size_t const to_write = strlen(files[i]) + 1;
        if (write(archive_fd, files[i], to_write) == -1) {
            fprintf(stderr, "error: could not write filenames to archive\n");
            status = 1;
            goto cleanup;
        }
    }


    cleanup:
    free(metadata);
    close(archive_fd);
    for (size_t i = 0; i < numfiles; i++) {
        if (fds[i] > 0) {
            close(fds[i]);
        }
    }
    free(fds);
    return status;
}

int unpack(const char *const archive_name, const char *const files[], int const numfiles) {
    printf("info: reading archive `%s`\n", archive_name);
    int const archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        fprintf(stderr, "error: could not open archive for reading: `%s`\n", archive_name);
        return 1;
    }
    struct btar_archive_header header;
    if (read(archive_fd, &header, sizeof(struct btar_archive_header))
         != sizeof(struct btar_archive_header)) {
        fprintf(stderr, "error: could not read archive header\n");
        close(archive_fd);
        return 1;
    }
    if (memcmp(header.magic, BTAR_MAGIC, BTAR_MAGIC_SIZE) != 0) {
        fprintf(stderr, "error: btar magic number does not match\n");
        close(archive_fd);
        return 1;
    }
    printf("`%s` is a valid btar archive, created at %s",
        archive_name, ctime((time_t *)(&header.creation_date)));
    printf("Archive contains %zu files and has a header length of %zu bytes\n",
            header.num_files, header.header_length);
    size_t const metadata_bytes = header.num_files * sizeof(struct btar_file_metadata);
    struct btar_file_metadata *const metadata = malloc(metadata_bytes);
    if (metadata == NULL) {
        fprintf(stderr, "error: could not allocate memory for metadata\n");
        close(archive_fd);
        return 1;
    }
    if ((size_t)read(archive_fd, metadata, metadata_bytes) != metadata_bytes) {
        fprintf(stderr, "error: could not read file metadata block\n");
        close(archive_fd);
        free(metadata);
        return 1;
    }
    size_t const filenames_bytes = header.header_length -
        (metadata_bytes + sizeof(struct btar_archive_header));

    char *const filenames_block = malloc(filenames_bytes);
    if (filenames_block == NULL) {
        fprintf(stderr, "error: could not allocate memory for filenames\n");
        close(archive_fd);
        free(metadata);
        return 1;
    }
    if ((size_t)read(archive_fd, filenames_block, filenames_bytes) != filenames_bytes) {
        fprintf(stderr, "error: could not read filenames in archive\n");
        close(archive_fd);
        free(metadata);
        free(filenames_block);
    }

    /// Print all files in archive
    printf("archive `%s` contains the following files\n", archive_name);
    char *filename = filenames_block;
    for (size_t i = 0; i < header.num_files; i++) {
        size_t const fname_len = strlen(filename);
        printf(" o `%s` (bytes=%zu, offset=%zu)\n",
            filename, metadata[i].num_bytes, metadata[i].archive_offset);
        filename += (fname_len + 1);
    }
}



static int help() {
    #define U(s) "\033[4m" s "\033[0m"
    #define B(s) "\033[1m" s "\033[0m"

    printf(B("btar -- usage\n"));
    printf("\no To create an archive from a set of files:\n");
    printf("  > btar pack " U("archive") " [" U("file1") " ... " U("fileN") "]\n");
    printf("\no To extract all (or certain) files from an archive:\n");
    printf("  > btar unpack " U("archive") " [" U("file1") " ... " U("fileN") "]\n");
    printf("\no To view the contents of an archive:\n");
    printf("  > btar list " U("archive") "\n");

    return 1;
}

int main(int const argc, const char *argv[]) {

    if (argc < 3) {
        return help();
    }
    if (strcmp(argv[1], "pack") == 0) {
        return pack(argv[2], &argv[3], (size_t)(argc - 3));
    }
    if (strcmp(argv[1], "unpack") == 0) {
        return unpack(argv[2], &argv[3], (size_t)(argc - 3));
    }
    if (strcmp(argv[1], "list") == 0) {
        return unpack(argv[2], NULL, 0);
    }
    return help();
}


