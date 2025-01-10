#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    char buffer[1024];
    int input_fd = open(argv[1], O_RDONLY);
    int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t bytes;
    while ((bytes = read(input_fd, buffer, 1024)) > 0) {
        write(output_fd, buffer, bytes);
    }
    close(input_fd);
    close(output_fd);
    return EXIT_SUCCESS;
}