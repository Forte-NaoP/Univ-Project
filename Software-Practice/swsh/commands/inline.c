#include "inline.h"

void cd(char *path) {
    if (chdir(path) == -1) {
        error_handler("cd", errno);
    }
}

char *pwd() {
    char *cwd = getcwd(NULL, 256);
    char *wd = concat_string(cwd, "\n", NULL);
    free(cwd);
    return wd;
}

void shell_exit(int status) {
    exit(status);
}
