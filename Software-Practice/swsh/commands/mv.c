#include "header_collection.h"

int main(int argc, char *argv[]) {
    if (rename(argv[1], argv[2]) == -1) {
        error_handler("mv", errno);
    }
}