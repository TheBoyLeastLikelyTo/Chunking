#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void write_chunk(FILE *input_file, FILE *output_file, size_t chunk_size) {
    char *buffer = (char *)malloc(chunk_size);
    size_t bytes_read = fread(buffer, 1, chunk_size, input_file);
    fwrite(buffer, 1, bytes_read, output_file);
    free(buffer);
}

void save_file_attributes(const char *file_path, const char *output_dir) {
    struct stat st;
    stat(file_path, &st);

    char attr_file_path[256];
    snprintf(attr_file_path, sizeof(attr_file_path), "%s/attributes.txt", output_dir);

    FILE *attr_file = fopen(attr_file_path, "w");
    if (attr_file == NULL) {
        perror("Failed to open attributes file");
        exit(EXIT_FAILURE);
    }

    fprintf(attr_file, "Size: %ld\n", st.st_size);
    fprintf(attr_file, "Mode: %o\n", st.st_mode);
    fclose(attr_file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path> <chunk_size> <output_dir>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *file_path = argv[1];
    size_t chunk_size = atoi(argv[2]);
    const char *output_dir = argv[3];

    FILE *input_file = fopen(file_path, "rb");
    if (input_file == NULL) {
        perror("Failed to open input file");
        return EXIT_FAILURE;
    }

    save_file_attributes(file_path, output_dir);

    char chunk_file_path[256];
    int chunk_index = 0;
    while (!feof(input_file)) {
        snprintf(chunk_file_path, sizeof(chunk_file_path), "%s/chunk_%d", output_dir, chunk_index);
        FILE *chunk_file = fopen(chunk_file_path, "wb");
        if (chunk_file == NULL) {
            perror("Failed to open chunk file");
            fclose(input_file);
            return EXIT_FAILURE;
        }

        write_chunk(input_file, chunk_file, chunk_size);
        fclose(chunk_file);
        chunk_index++;
    }

    fclose(input_file);
    return EXIT_SUCCESS;
}
