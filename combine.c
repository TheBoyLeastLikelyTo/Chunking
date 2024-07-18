#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void read_chunk(FILE *chunk_file, FILE *output_file) {
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), chunk_file)) > 0) {
        fwrite(buffer, 1, bytes_read, output_file);
    }
}

void restore_file_attributes(const char *output_file_path, const char *input_dir) {
    char attr_file_path[256];
    snprintf(attr_file_path, sizeof(attr_file_path), "%s/attributes.txt", input_dir);

    struct stat st;
    FILE *attr_file = fopen(attr_file_path, "r");
    if (attr_file == NULL) {
        perror("Failed to open attributes file");
        exit(EXIT_FAILURE);
    }

    size_t file_size;
    int file_mode;
    fscanf(attr_file, "Size: %ld\n", &file_size);
    fscanf(attr_file, "Mode: %o\n", &file_mode);
    fclose(attr_file);

    chmod(output_file_path, file_mode);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_dir> <output_file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_dir = argv[1];
    const char *output_file_path = argv[2];

    FILE *output_file = fopen(output_file_path, "wb");
    if (output_file == NULL) {
        perror("Failed to open output file");
        return EXIT_FAILURE;
    }

    char chunk_file_path[256];
    int chunk_index = 0;
    FILE *chunk_file;
    while (1) {
        snprintf(chunk_file_path, sizeof(chunk_file_path), "%s/chunk_%d", input_dir, chunk_index);
        chunk_file = fopen(chunk_file_path, "rb");
        if (chunk_file == NULL) {
            break;
        }

        read_chunk(chunk_file, output_file);
        fclose(chunk_file);
        chunk_index++;
    }

    fclose(output_file);

    restore_file_attributes(output_file_path, input_dir);

    return EXIT_SUCCESS;
}
