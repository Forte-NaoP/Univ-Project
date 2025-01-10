#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }
    if (unlink(argv[1]) == -1) {
        error_handler("rm", errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}