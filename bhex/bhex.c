

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>

#define STRINGS_BUFF_SIZE 4096
#define MIN_STRING_LENGTH 4

#define XXD_BYTES_PER_LINE 16

#define MIN(A, B) ((A) < (B) ? (A) : (B))

static int usage() {
    printf("bhex [xxd|strings] <file>\n");
    return 1;
}

static int is_printable(char const byte) {
    return byte >= ' ' && byte <= '~';
}

static char representation(char const byte) {
    if (is_printable(byte)) {
        return byte;
    } else {
        return '.';
    }
}

static void output_xxd_line(
    uint64_t const offset,
    const char *const line,
    uint64_t const num_bytes) {

    assert(num_bytes <= XXD_BYTES_PER_LINE);

    printf("%llx: ", offset);
    for (uint64_t i = 0; i < num_bytes; i++) {
        printf("%02hhx ", line[i]);
    }
    printf("%*c", (int)(XXD_BYTES_PER_LINE - num_bytes), ' ');
    for (uint64_t i = 0; i < num_bytes; i++) {
        printf("%c", representation(line[i]));
    }
}

static int xxd(int const fd) {
    FILE *const stream = fdopen(fd, "r");
    if (stream == NULL) {
        fprintf(stderr, "could not convert file to stream\n");
        close(fd);
        return 1;
    }
    char buff[XXD_BYTES_PER_LINE];
    uint64_t file_offset = 0;
    uint64_t line_offset = 0;
    uint64_t line_index = 0;
    int byte = -1;
    while ((byte = fgetc(stream)) >= 0) {
        if (line_index == XXD_BYTES_PER_LINE) {
            line_index = 0;
            output_xxd_line(line_offset, buff, XXD_BYTES_PER_LINE);
            printf("\n");
            line_offset = file_offset;
        }
        buff[line_index] = (char)byte;
        line_index++;
        file_offset++;
    }
    if (line_index != 0) {
        output_xxd_line(line_offset, buff, XXD_BYTES_PER_LINE);
        printf("\n");
    }
    return 0;
}

static int strings(int const fd) {
    char saved_string[MIN_STRING_LENGTH + 1];
    saved_string[MIN_STRING_LENGTH] = '\0';

    size_t current_string_length = 0;
    uint64_t string_start_offset = 0; // offset in file
    int in_string = 0;

    char buff[STRINGS_BUFF_SIZE];
    ssize_t bytes_read = -1;
    uint64_t file_offset = 0;

    while ((bytes_read = read(fd, buff, STRINGS_BUFF_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            file_offset++;
            char const c = buff[i];

            if (is_printable(c)) {
                if (!in_string) {
                    string_start_offset = file_offset;
                    current_string_length = 0;
                    in_string = 1;
                }
                // We have reached our minimum number of consecutive
                // ascii characters. Now we can print out the strings start
                if (current_string_length == MIN_STRING_LENGTH - 1) {
                    saved_string[current_string_length] = c;
                    current_string_length++;
                    printf("0x%llx: %s", string_start_offset, saved_string);
                }
                // The string is continued, so we just keep printing its chars
                else if (current_string_length >= MIN_STRING_LENGTH) {
                    printf("%c", c);
                }
                // We are at the start of a string maybe? let's keep track
                else {
                    saved_string[current_string_length] = c;
                    current_string_length++;
                }

            } else {
                // End the current string (and print it if it was long enough)
                if (in_string && current_string_length >= MIN_STRING_LENGTH) {
                    printf("\n");
                }
                in_string = 0;
                current_string_length = 0;
            }
        }
    }
    close(fd);
    if (bytes_read == -1) {
        fprintf(stderr, "read error\n");
        return 1;
    }
    if (current_string_length > 0 &&
        current_string_length < MIN_STRING_LENGTH) {
        saved_string[current_string_length] = '\0';
        printf("0x%llx: %s\n", string_start_offset, saved_string);
    } else if (current_string_length >= MIN_STRING_LENGTH) {
        printf("\n");
    }
    return 0;
}

int main(int const argc, const char *const argv[]) {
    if (argc != 3) {
        return usage();
    }

    const char *const subcommand = argv[1];
    const char *const filename = argv[2];

    int const fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "error: could not open file: %s\n", filename);
        return 1;
    }

    if (strcmp(subcommand, "xxd") == 0) {
        return xxd(fd);
    } else if (strcmp(subcommand, "strings") == 0) {
        return strings(fd);
    } else {
        close(fd);
        return usage();
    }
}