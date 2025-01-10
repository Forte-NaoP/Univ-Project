#include "error_handler.h"

void error_handler(char *cmd, int err_num) {
    char *str = NULL;
    char *int_str = NULL;
    switch (err_num) {
        case EACCES:
            str = concat_string(cmd, ": Permission denied\n", NULL);
            break;
        case EISDIR:
            str = concat_string(cmd, ": Is a directory\n", NULL);
            break;
        case ENOENT:
            str = concat_string(cmd, ": No such file or directory\n", NULL);
            break;
        case ENOTDIR:
            str = concat_string(cmd, ": Not a directory\n", NULL);
            break;
        case EPERM:
            str = concat_string(cmd, ": Permission denied\n", NULL);
            break;
        default:
            int_str = int2str(NULL, err_num);
            str = concat_string(cmd, ": Error occurred: ", int_str, "\n", NULL);
            break;
    }
    write(STDERR_FILENO, str, strlen(str));
    if (int_str != NULL) free(int_str);
    free(str);
}