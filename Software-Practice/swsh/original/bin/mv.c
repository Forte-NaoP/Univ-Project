#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "err.h"

int mv(char *name, char *target)
{
    int in, out;
    int i, len;
    char *tmp1, *tmp2;
    char buff[100];

    tmp1=strrchr(name,'/');
    tmp2=strrchr(target,'/');

    if((tmp1==NULL && tmp2==NULL)){
        if(rename(name,target)==-1){
            err_handle("mv",errno);
        }
        return 0;
    }
    if((tmp1-name)==(tmp2-target)){

        len=tmp1-name;
        for(i=0;i<=len;i++){
            if(name[i]!=target[i]) break;
        }
        if(i==len){
            if(rename(name,target)==-1){
                err_handle("mv",errno);
            }
        }
        else{
            in=open(name, O_RDONLY);
            out=open(target, O_WRONLY|O_CREAT|O_TRUNC,0644);
            while(0<(len=read(in,buff,100))){
                write(out,buff,len);
            }
        }
    }
    else{
        in=open(name, O_RDONLY);
        out=open(target, O_WRONLY|O_CREAT|O_TRUNC,0644);
        while(0<(len=read(in,buff,100))){
            write(out,buff,len);
        }
    }

    close(in);
    close(out);
}

int main(int argc, char *argv[])
{
    mv(argv[1],argv[2]);
}
