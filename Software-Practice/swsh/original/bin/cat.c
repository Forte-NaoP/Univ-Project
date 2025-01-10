#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "err.h"

int cat(char *name)
{
    char buf[10000];
    int fd;
    if(name==NULL){
      while(read(0,buf,10000)>0){
          printf("%s", buf);
      }
    }
    else{
      fd=open(name,O_RDONLY);
      while(read(fd,buf,10000)>0){
          printf("%s", buf);
      }
      close(fd);
    }

}

int main(int argc, char *argv[])
{
    cat(argv[1]);
}
