#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (unlink(argv[1]) == -1) {
        error_handler("rm", errno);
    }
}