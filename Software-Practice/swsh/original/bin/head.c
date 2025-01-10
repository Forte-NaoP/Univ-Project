#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "../swsh_header.h"
#include "err.h"


int head(char *name, char *op_1, char *op_2)
{
    char buff[1000]={};
    int i,cnt;
    int size=1;
    int fd;

    if(name==NULL){
        if(op_1==NULL) cnt=10;
        else{
            cnt=atoi(op_2);
        }
        while(size>0){
            size=read(STDIN_FILENO,buff,1000);
            if(size<1) break;
            for(i=0;i<size;i++){
                if(buff[i]=='\n') cnt--;
                if(cnt==0) break;
            }
            if(i==size){
                printf("%s", buff);
            }
            else{
                buff[i+1]='\0';
                printf("%s", buff);
                break;
            }
        }
    }
    else{
        fd=open(name, O_RDONLY);
        if(op_1==NULL) cnt=10;
        else{
            cnt=atoi(op_2);
        }
        while(size>0){
            size=read(fd,buff,1000);
            if(size<1) break;
            for(i=0;i<size;i++){
                if(buff[i]=='\n') cnt--;
                if(cnt==0) break;
            }
            if(i==size){
                printf("%s", buff);
            }
            else{
                buff[i+1]='\0';
                printf("%s", buff);
                break;
            }
        }
        close(fd);
    }

}


int main(int argc, char *argv[])
{
    if(argc<=2){
        head(argv[1],NULL, NULL);
    }
    else{
        if(argc==3) head(NULL,argv[1], argv[2]);
        else head(argv[3],argv[1], argv[2]);
    }
}
