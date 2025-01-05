#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "swsh_header.h"
#include "./bin/err.h"

int cd(char *name)
{
    //printf("%s\n", name);
    if(name==NULL){
        if(chdir(getenv("HOME"))==-1){
            err_handle("cd", errno);
        }
    }
    if(chdir(name)==-1){
        err_handle(name, errno);
    }
    return 0;
}

int my_pwd(char *name)
{
    char buf[300];
    getcwd(buf,200);
    printf("%s\n",buf);
    return 0;
}

int my_exit(char *num)
{
    int x=0;
    if(num!=NULL) x=atoi(num);
    exit(x);
}
