#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void split_file(const char *file_path, const char *output_folder, size_t max_chunk_size);
void split_folder(const char *folder_path, const char *output_folder, size_t max_chunk_size);
void write_chunk(const char *output_folder, const char *file_name, int chunk_index, char *buffer, size_t buffer_size);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file/folder path> <max chunk size> <output folder>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    size_t max_chunk_size = strtoul(argv[2], NULL, 10);
    const char *output_folder = argv[3];

    struct stat path_stat;
    stat(input_path, &path_stat);

    mkdir(output_folder, 0777); // Create the output folder if it doesn't exist

    if (S_ISDIR(path_stat.st_mode)) {
        split_folder(input_path, output_folder, max_chunk_size);
    } else {
        split_file(input_path, output_folder, max_chunk_size);
    }

    return 0;
}

void split_file(const char *file_path, const char *output_folder, size_t max_chunk_size) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    int chunk_index = 0;
    char *file_name = strrchr(file_path, '/');
    file_name = (file_name) ? file_name + 1 : (char *)file_path; // Get the file name from the path

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytes_read; i += max_chunk_size) {
            size_t chunk_size = (i + max_chunk_size < bytes_read) ? max_chunk_size : bytes_read - i;
            write_chunk(output_folder, file_name, chunk_index++, buffer + i, chunk_size);
        }
    }

    fclose(file);
}

void split_folder(const char *folder_path, const char *output_folder, size_t max_chunk_size) {
    DIR *dir = opendir(folder_path);
    if (!dir) {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // If the entry is a regular file
            char file_path[BUFFER_SIZE];
            snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);
            split_file(file_path, output_folder, max_chunk_size);
        }
    }

    closedir(dir);
}

void write_chunk(const char *output_folder, const char *file_name, int chunk_index, char *buffer, size_t buffer_size) {
    char output_path[BUFFER_SIZE];
    snprintf(output_path, sizeof(output_path), "%s/%s_%d.chunk", output_folder, file_name, chunk_index);

    FILE *chunk_file = fopen(output_path, "wb");
    if (!chunk_file) {
        perror("Unable to open chunk file for writing");
        return;
    }

    fwrite(buffer, 1, buffer_size, chunk_file);
    fclose(chunk_file);
}
