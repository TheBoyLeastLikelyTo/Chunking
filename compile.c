#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

void split_file(const char *file_path, long max_chunk_size, const char *output_dir);
void split_folder(const char *folder_path, long max_chunk_size, const char *output_dir);
void write_chunk(FILE *source, FILE *dest, long chunk_size);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_or_folder> <max_chunk_size> <output_dir>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    long max_chunk_size = atol(argv[2]);
    const char *output_dir = argv[3];

    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        perror("stat");
        return 1;
    }

    if (S_ISREG(path_stat.st_mode)) {
        split_file(path, max_chunk_size, output_dir);
    } else if (S_ISDIR(path_stat.st_mode)) {
        split_folder(path, max_chunk_size, output_dir);
    } else {
        fprintf(stderr, "Invalid path: %s\n", path);
        return 1;
    }

    return 0;
}

void split_file(const char *file_path, long max_chunk_size, const char *output_dir) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        return;
    }

    char output_file_path[BUFFER_SIZE];
    long bytes_read = 0;
    int chunk_number = 0;

    while (!feof(file)) {
        snprintf(output_file_path, BUFFER_SIZE, "%s/chunk_%03d", output_dir, chunk_number);
        FILE *chunk = fopen(output_file_path, "wb");
        if (!chunk) {
            perror("fopen");
            fclose(file);
            return;
        }

        write_chunk(file, chunk, max_chunk_size);
        fclose(chunk);
        chunk_number++;
    }

    fclose(file);
}

void split_folder(const char *folder_path, long max_chunk_size, const char *output_dir) {
    DIR *dir = opendir(folder_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char file_path[BUFFER_SIZE];
    char output_file_path[BUFFER_SIZE];
    long bytes_read = 0;
    int chunk_number = 0;
    FILE *current_chunk = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            snprintf(file_path, BUFFER_SIZE, "%s/%s", folder_path, entry->d_name);
            FILE *file = fopen(file_path, "rb");
            if (!file) {
                perror("fopen");
                continue;
            }

            while (!feof(file)) {
                if (!current_chunk || bytes_read >= max_chunk_size) {
                    if (current_chunk) fclose(current_chunk);
                    snprintf(output_file_path, BUFFER_SIZE, "%s/chunk_%03d", output_dir, chunk_number);
                    current_chunk = fopen(output_file_path, "wb");
                    if (!current_chunk) {
                        perror("fopen");
                        fclose(file);
                        closedir(dir);
                        return;
                    }
                    bytes_read = 0;
                    chunk_number++;
                }

                long chunk_size = max_chunk_size - bytes_read;
                write_chunk(file, current_chunk, chunk_size);
                bytes_read += chunk_size;
            }

            fclose(file);
        }
    }

    if (current_chunk) fclose(current_chunk);
    closedir(dir);
}

void write_chunk(FILE *source, FILE *dest, long chunk_size) {
    char buffer[BUFFER_SIZE];
    long bytes_to_read = chunk_size;
    while (bytes_to_read > 0 && !feof(source)) {
        size_t bytes_read = fread(buffer, 1, bytes_to_read > BUFFER_SIZE ? BUFFER_SIZE : bytes_to_read, source);
        fwrite(buffer, 1, bytes_read, dest);
        bytes_to_read -= bytes_read;
    }
}
