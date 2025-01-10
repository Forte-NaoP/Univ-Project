#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "err.h"

int rm(char *name)
{
    if(unlink(name)==-1){
        err_handle("rm",errno);
    }
}

int main(int argc, char *argv[])
{
    rm(argv[1]);
}
