#include "header_collection.h"

int main(int argc, char *argv[]) {
    char buffer[1024];
    int line = argc > 2 ? atoi_32(argv[2]) : 10;
    int fd = argc > 2 ? open(argv[argc - 1], O_RDONLY) : open(argv[1], O_RDONLY);
    ssize_t bytes;
    int cur = 0, idx = 0;
    while ((bytes = read(fd, buffer, 1024)) > 0) {
        for (idx = 0; idx < bytes && cur < line; ++idx) {
            if (buffer[idx] == '\n') {
                ++cur;
            }
        }
        write(STDOUT_FILENO, buffer, idx + 1);
        if (cur == line) break;
    }
    close(fd);
}