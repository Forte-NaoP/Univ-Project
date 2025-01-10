#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    if (rename(argv[1], argv[2]) == -1) {
        error_handler("mv", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}