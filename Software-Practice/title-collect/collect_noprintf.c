//-----------------------------------------------------------
// 2016313621 BAE JUN HWI
//
// SWE2007: Software Experiment II (Fall 2017)
//
// Skeleton code for PA //2
// September 27, 2017
//
// Jong-Won Park
// Embedded Software Laboratory
// Sungkyunkwan University
//
//-----------------------------------------------------------

#include "mystring.h"
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<stdlib.h>
#include<wait.h>
#include<fcntl.h>
#include<stdio.h>

typedef struct get{
    char name[100];
    char title[1000];
    int size;
}GET;

void collect()
{
    char buff[10000]={};
    char *dtok[1000];
    char *ttok[1000];
    char result[1000]={};
    char i2s[12]={};
    char title[1000]={};
    char *c;
    char option[6][10]={"http://", "https://", "print", "stat", "load", "quit"};
    int len=0, size=0;
    int status, pc, order=0, stat_cnt=0;
    int i,j,k;
    int fd,stin=0;
    pid_t pid;
    GET dom[1000];

    //signal(SIGINT,SIG_IGN);
    while(1){
      //init
        memset(ttok,0,sizeof(ttok));
        memset(dtok,0,sizeof(dtok));
        memset(result,0,sizeof(result));
        memset(i2s,0,sizeof(i2s));
        memset(title,0,sizeof(title));

        if(stin==0){
            scanf("%s", buff);
        }
        else{
            read(stin,buff,10000);
            c=strpbrk(buff,"\n");
            *c='\0';
        }
        //for(i=0;i<(buff-c);i++) printf("%c ", buff[i]);
        //printf("%s\n", buff);
        //printf("\n");

        //wget routine
        if(strstr(buff,option[0])!=NULL || strstr(buff,option[1])!=NULL ){
            pc++;
            int2str(i2s,pc);
            if((pid=fork())==0){
                execlp("wget", "wget", "-q", "-O", i2s, buff,(char *)0);
            }
            else{
                wait(&status);
                if((fd=open(i2s,O_RDONLY))<0){
                    /*strcat(result,i2s);
                    strcat(result,">");
                    strcat(result,"Error occurred!\n");
                    write(1,result,strlen(result));*/
                    printf("%d>Error occurred!\n",pc);
                }
                else{

                    //Main domain
                    i=-1;
                    dtok[++i]=strpbrk(buff,".");
                    while(dtok[i]!=NULL){
                        dtok[i+1]=strpbrk(dtok[i]+1,".");
                        i++;
                    }
                    i--;
                    strtok(dtok[i],"/");
                    if(strcmp(dtok[i],".kr")==0){
                        strcpy(result,dtok[i-2]+1);
                        i-=2;
                    }
                    else{
                        strcpy(result,dtok[i-1]+1);
                        i-=1;
                    }
                    printf("domain: %s\n", result);
                    //title
                    read(fd,buff,10000);
                    ttok[0]=strstr(buff,"<title>");
                    ttok[1]=strstr(ttok[0]+7,"</title>");

                    memcpy(title,ttok[0]+7,ttok[1]-ttok[0]-7);
                    title[ttok[1]-ttok[0]-7]='\0';
                    close(fd);

                    for(j=0;j<size;j++){
                        if(strcmp(dom[j].name,result)==0){
                            len=(ttok[1]-ttok[0]-7);
                            if(dom[j].size<len){
                                memcpy(dom[j].title,title,len);
                                dom[j].title[len]='\0';
                                dom[j].size=len;
                                break;
                            }
                        }
                    }
                    if(j==size){
                        len=(ttok[1]-ttok[0]-7);
                        memcpy(dom[j].title,title,len);
                        dom[j].title[len]='\0';
                        strcpy(dom[j].name,result);
                        dom[j].size=len;
                        size++;
                    }

                    printf("%d>%s:%s\n", pc, dom[size-1].name,dom[size-1].title);
                }
            }
        }//wget routine end
        else if(strstr(buff,option[2])!=NULL){ //print routine
            scanf("%s\n", buff);
            pc++;
            //int2str(i2s,pc);
            //ttok[0]=strtok(buff," ");
            for(i=0;i<size;i++){
                if(strcmp(ttok[0],dom[i].name)==0){
                    write(1,i2s,13);
                    write(1,">",2);
                    write(1,dom[i].title,size);
                    write(1,"\n",2);
                    break;
                }
            }
            if(i==size){
                write(1,i2s,13);
                write(1,">",2);
                write(1,"Not Available\n",15);
            }
        }
        else if(strstr(buff,option[3])!=NULL){ //stat routine
            pc++;
            int2str(i2s,pc);
            write(1,i2s,13);
            write(1,">",2);
            memset(i2s,0,sizeof(i2s));
            int2str(i2s,stat_cnt);
            write(1,i2s,13);
            write(1," titles",8);
        }
        else if(strstr(buff,option[4])!=NULL){ //load routine
            pc++;
            ttok[0]=strtok(buff," ");
            ttok[0]=strtok(NULL," ");
            pid=fork();
            if(pid==0){
                stin=open(ttok[0],O_RDONLY);
            }
            else{
                wait(&status);
            }
        }
        else if(strstr(buff,option[5])!=NULL){ //quit routine
            kill(pid,SIGKILL);
            exit(0);
        }
        else{ //input error
            //do nothing
        }
    }
}

int main(int argc, char* argv[])
{
    collect();
    return 0;
}
