#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void reconstitute_chunks(const char *input_folder, const char *output_folder);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input folder> <output folder>\n", argv[0]);
        return 1;
    }

    const char *input_folder = argv[1];
    const char *output_folder = argv[2];

    mkdir(output_folder, 0777); // Create the output folder if it doesn't exist

    reconstitute_chunks(input_folder, output_folder);

    return 0;
}

void reconstitute_chunks(const char *input_folder, const char *output_folder) {
    DIR *dir = opendir(input_folder);
    if (!dir) {
        perror("Unable to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".chunk")) { // If the entry is a chunk file
            char file_path[BUFFER_SIZE];
            snprintf(file_path, sizeof(file_path), "%s/%s", input_folder, entry->d_name);

            // Extract the original file name and chunk index from the chunk file name
            char original_file_name[BUFFER_SIZE];
            int chunk_index;
            sscanf(entry->d_name, "%[^_]_%d.chunk", original_file_name, &chunk_index);

            char output_path[BUFFER_SIZE];
            snprintf(output_path, sizeof(output_path), "%s/%s", output_folder, original_file_name);

            FILE *output_file = fopen(output_path, "ab");
            if (!output_file) {
                perror("Unable to open output file for writing");
                continue;
            }

            FILE *chunk_file = fopen(file_path, "rb");
            if (!chunk_file) {
                perror("Unable to open chunk file for reading");
                fclose(output_file);
                continue;
            }

            char buffer[BUFFER_SIZE];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), chunk_file)) > 0) {
                fwrite(buffer, 1, bytes_read, output_file);
            }

            fclose(chunk_file);
            fclose(output_file);
        }
    }

    closedir(dir);
}
