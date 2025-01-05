#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "err.h"

void cp(char *file1, char *file2)
{
    char buf[1000];
    int ret;
    int in, out;
    //printf("%s\n%s\n", file1, file2);
    if((in=open(file1,O_RDONLY))==-1){
        printf("in fail\n");
    }
    if((out=open(file2,O_WRONLY|O_CREAT|O_TRUNC,0644))==-1){
        printf("out fail\n");
        exit(0);
    }

    while((ret=read(in,buf,1000))>0){
        write(out,buf,ret);
    }
    close(in);
    close(out);
}

//cat < /proc/meminfo | grep -i active | tail -n 4 > memory.txt

int main(int argc, char **argv)
{
    cp(argv[1],argv[2]);
}
