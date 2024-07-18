#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int is_chunk_file(const char *filename);
int compare_chunk_files(const void *a, const void *b);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <chunks_directory>\n", argv[0]);
        return 1;
    }

    const char *chunks_directory = argv[1];
    DIR *dir = opendir(chunks_directory);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    char **chunk_files = NULL;
    size_t chunk_count = 0;

    // Read chunk files
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_chunk_file(entry->d_name)) {
            chunk_files = realloc(chunk_files, (chunk_count + 1) * sizeof(char *));
            chunk_files[chunk_count] = strdup(entry->d_name);
            chunk_count++;
        }
    }

    closedir(dir);

    if (chunk_count == 0) {
        fprintf(stderr, "No chunk files found in the directory.\n");
        return 1;
    }

    // Sort chunk files
    qsort(chunk_files, chunk_count, sizeof(char *), compare_chunk_files);

    char output_file_path[BUFFER_SIZE];
    snprintf(output_file_path, BUFFER_SIZE, "%s/recompiled_output", chunks_directory);
    FILE *output_file = fopen(output_file_path, "wb");
    if (!output_file) {
        perror("fopen");
        for (size_t i = 0; i < chunk_count; i++) {
            free(chunk_files[i]);
        }
        free(chunk_files);
        return 1;
    }

    char file_path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    for (size_t i = 0; i < chunk_count; i++) {
        snprintf(file_path, BUFFER_SIZE, "%s/%s", chunks_directory, chunk_files[i]);
        FILE *chunk_file = fopen(file_path, "rb");
        if (!chunk_file) {
            perror("fopen");
            continue;
        }

        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, chunk_file)) > 0) {
            fwrite(buffer, 1, bytes_read, output_file);
        }

        fclose(chunk_file);
        free(chunk_files[i]);
    }

    free(chunk_files);
    fclose(output_file);

    printf("Recompiled file created: %s\n", output_file_path);

    return 0;
}

int is_chunk_file(const char *filename) {
    return strncmp(filename, "chunk_", 6) == 0 && isdigit(filename[6]) && isdigit(filename[7]) && isdigit(filename[8]);
}

int compare_chunk_files(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}
