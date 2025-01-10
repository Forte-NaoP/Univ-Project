#include "header_collection.h"

int main(int argc, char *argv[]) {
    char buffer[1024];
    int line = argc > 2 ? atoi_32(argv[2]) : 10;
    int fd = argc > 2 ? open(argv[argc - 1], O_RDONLY) : open(argv[1], O_RDONLY);
    ssize_t bytes;
    off_t file_size = lseek(fd, 0, SEEK_END);
    off_t offset = file_size;

    while (offset > 0 && line > 0) {
        ssize_t chunk = offset >= 1024 ? 1024 : offset;
        offset -= chunk;
        lseek(fd, offset, SEEK_SET);
        bytes = read(fd, buffer, chunk);
        for (ssize_t i = bytes - 1; i >= 0; --i) {
            if (buffer[i] == '\n') {
                --line;
                if (line == 0) {
                    lseek(fd, offset + i + 1, SEEK_SET);
                    break;
                }
            }
        }
    }

    if (offset == 0 && line > 0) {
        lseek(fd, 0, SEEK_SET);
    }

    while ((bytes = read(fd, buffer, 1024)) > 0) {
        write(STDOUT_FILENO, buffer, bytes);
    }

    close(fd);
    return 0;
}