#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"

void err_handle(char *cmd, int err_num)
{
    int fd;
    fd=dup(STDOUT_FILENO);
    dup2(STDERR_FILENO, STDOUT_FILENO);

    if(err_num==EACCES){
        printf("%s: Permission denied\n", cmd);
    }
    else if(err_num==EISDIR){
        printf("%s: Is a directory\n", cmd);
    }
    else if(err_num==ENOENT){
        printf("%s: No Such file or directory\n", cmd);
    }
    else if(err_num==ENOTDIR){
        printf("%s: Not a directory\n", cmd);
    }
    else if(err_num==EPERM){
        printf("%s: Permission denied\n", cmd);
    }
    else{
        printf("%s: Error occurred: %d\n", cmd, err_num);
    }
    dup2(fd,STDOUT_FILENO);
}
