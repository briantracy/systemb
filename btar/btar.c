
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "btar.h"


/*

+--------
|


*/

/// btar only accepts files in the current directory
static int is_valid_filename(const char *const name) {
    return strlen(name) < BTAR_FILENAME_SIZE - 1 &&
            strchr(name, '/') == NULL;
}

/// Create an archive
int pack(const char *const archive_name, const char *const files[], int const numfiles) {
    printf("info: creating archive `%s` from `%d` files\n", archive_name, numfiles);

    struct btar_archive_header header;
    memcpy(header.magic, BTAR_MAGIC, BTAR_MAGIC_SIZE);
    header.creation_date = time(NULL);
    header.num_files = numfiles;


    int status = 0;

    // File descriptors to read from
    int *const fds = calloc(numfiles, sizeof(int));
    struct btar_file_metadata *const metadata = calloc(numfiles, sizeof(struct btar_file_metadata));
    if (fds == NULL || metadata == NULL) {
        fprintf(stderr, "error: could not allocate space\n");
        status = 1;
        goto cleanup;
    }
    for (int i = 0; i < numfiles; i++) {
        printf("info: opening file `%s`\n", files[i]);
        if (!is_valid_filename(files[i])) {
            fprintf(stderr, "error: file name `%s` too long not in current directory\n", files[i]);
            status = 1;
            goto cleanup;
        }
        fds[i] = open(files[i], O_RDONLY);
        if (fds[i] == -1) {
            fprintf(stderr, "error: could not open file `%s` for reading\n", files[i]);
            status = 1;
            goto cleanup;
        }
        // memset(&(metadata[i].name), 0, BTAR_FILENAME_SIZE);
        // strncpy(&(metadata[i].name), files[i], BTAR_FILENAME_SIZE - 1);
    }

    int const archive_fd = open(archive_name, O_WRONLY | O_CREAT | O_EXCL, 0444);
    if (archive_fd == -1) {
        fprintf(stderr, "error: could not create archive file\n");
        status = 1;
        goto cleanup;
    }

    int s = write(archive_fd, &header, sizeof(struct btar_archive_header));
    if (s == -1) {
        fprintf(stderr, "error: could not write archive header to file\n");
        status = 1;
        goto cleanup;
    }
    s = write(archive_fd, metadata, sizeof(struct btar_file_metadata) * numfiles);
    if (s == -1) {
        fprintf(stderr, "error: could not write metadata blocks to archive\n");
        status = 1;
        goto cleanup;
    }


    /// At this point, all files are open and ready to be added to the archive

    for (int i = 0; i < numfiles; i++) {
        struct btar_file_metadata *const md = &metadata[i];

    }


    cleanup:
    free(metadata);
    for (int i = 0; i < numfiles; i++) {
        if (fds[i] > 0) {
            close(fds[i]);
        }
    }
    free(fds);
    return status;
}
int unpack(const char *archive_name, const char *files[], int const numfiles) {
    (void)archive_name; (void)files; (void)numfiles;
    return 1;
}





static int help() {
    #define U(s) "\033[4m" s "\033[0m"

    printf("\033[1mbtar -- usage\033[0m\n");
    printf("\n o To create an archive from a set of files:\n");
    printf("  > btar pack " U("archive") " [" U("file1") " ... " U("fileN") "]\n");
    printf("\n o To extract all (or certain) files from an archive:\n");
    printf("  > btar unpack " U("archive") " [" U("file1") " ... " U("fileN") "]\n");
    printf("\n o To view the contents of an archive:\n");
    printf("  > btar list " U("archive") "\n");

    return 1;
}

int main(int const argc, const char *argv[]) {

    if (argc < 3 || (strcmp(argv[1], "pack") != 0 && strcmp(argv[1], "unpack") != 0)) {
        return help();
    }
    if (strcmp(argv[1], "pack") == 0) {
        return pack(argv[2], &argv[3], argc - 3);
    }
    if (strcmp(argv[1], "unpack") == 0) {
        return unpack(argv[2], &argv[3], argc - 3);
    }
    return help();
}


