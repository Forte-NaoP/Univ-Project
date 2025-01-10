#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "err.h"


int tail(char *name, char *op_1, char *op_2)
{
    char buff[1000]={};
    int i,cnt,tmp=0, sign=0, k;
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
                if(buff[i]=='\n'){
                    tmp++;
                }
            }
        }
        k=tmp-cnt;
        lseek(STDIN_FILENO,-1,SEEK_END);
        read(STDIN_FILENO,buff,2);
        if(buff[0]!='\n') k++;

        memset(buff,0,1000);
        lseek(STDIN_FILENO,0,SEEK_SET);
        size=1;
        while((size=read(STDIN_FILENO,buff,1000))>0){
            if(k==0) printf("%s", buff);
            if(k!=0){
                for(i=0;i<size;i++){
                    if(buff[i]=='\n'){
                        k--;
                        if(k==0){
                            printf("%s", buff+i+1);
                            break;
                        }
                    }
                }
            }
            memset(buff,0,1000);
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
                if(buff[i]=='\n'){
                    tmp++;
                }
            }
        }
        k=tmp-cnt;
        lseek(fd,-1,SEEK_END);
        read(fd,buff,2);
        if(buff[0]!='\n') k++;

        memset(buff,0,1000);
        lseek(fd,0,SEEK_SET);
        size=1;
        while((size=read(fd,buff,1000))>0){
            if(k==0) printf("%s", buff);
            if(k!=0){
                for(i=0;i<size;i++){
                    if(buff[i]=='\n'){
                        k--;
                        if(k==0){
                            printf("%s", buff+i+1);
                            break;
                        }
                    }
                }
            }
            memset(buff,0,1000);
        }
        close(fd);
    }

}


int main(int argc, char *argv[])
{
    if(argc<=2){
        tail(argv[1],NULL, NULL);
    }
    else{
        if(argc==3) tail(NULL,argv[1], argv[2]);
        else tail(argv[3],argv[1], argv[2]);
    }
}
