#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    char buffer[1024];
    int fd = open(argv[1], O_RDONLY);
    ssize_t bytes;
    while ((bytes = read(fd, buffer, 1024)) > 0) {
        write(1, buffer, bytes);
    }
    close(fd);
    return EXIT_SUCCESS;
}