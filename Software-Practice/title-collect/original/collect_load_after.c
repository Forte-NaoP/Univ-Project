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
#include<wait.h>
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>

#define D_KEY_NUM 9999
#define I_KEY_NUM 6666
#define Z_KEY_NUM 7777
#define S_KEY_NUM 8888
#define dom_sz 1104000

typedef struct get{
    char name[100];
    char title[1000];
    int size;
}GET;

GET *dom;
int *pc;
int *stat_cnt;
int *size;
int d_shmid, i_shmid, s_shmid, z_shmid;

void init()
{
    d_shmid=shmget((key_t)D_KEY_NUM, dom_sz, IPC_CREAT|0666);
    dom=(GET *)shmat(d_shmid, (void *)0, 0);

    i_shmid=shmget((key_t)I_KEY_NUM, sizeof(int), IPC_CREAT|0666);
    pc=(int *)shmat(i_shmid, (void *)0, 0);

    s_shmid=shmget((key_t)S_KEY_NUM, sizeof(int), IPC_CREAT|0666);
    stat_cnt=(int *)shmat(s_shmid, (void *)0, 0);

    z_shmid=shmget((key_t)Z_KEY_NUM, sizeof(int), IPC_CREAT|0666);
    size=(int *)shmat(z_shmid, (void *)0, 0);
}

void collect()
{
    char buff[10000]={};
    char t_buff[10000];
    char *dtok[1000];
    char *ttok[1000];
    char *btok[1000];
    char result[1000]={};
    char i2s[12]={};
    char title[1000]={};
    char option[6][10]={"http://", "https://", "print", "stat", "load", "quit"};

    int len=0, t_size;
    int status=0, order=0;
    int i,j,load_cnt=0,k=0, sign=0;
    int fd,stin=0;
    pid_t pid;

    init();
    signal(SIGINT,SIG_IGN);
    memset(dom,0,sizeof(GET)*1000);
    (*stat_cnt)=0;
    (*pc)=0;
    while(1){
      //init
        memset(result,0,sizeof(result));
        memset(i2s,0,sizeof(i2s));
        memset(title,0,sizeof(title));
        if(stin==0){
            scanf("%[^\n]", buff);
            btok[0]=buff;
            getchar();
        }
        else if(sign==0){
            memset(buff,0,10000);
            read(stin,buff,10000);
            load_cnt=0;
            k=-1;
            btok[++k]=strtok(buff,"\n");
            while(btok[k]!=NULL){
                btok[k+1]=strtok(NULL,"\n");
                k++;
            }
            k--;
            sign=1;
        }
        else{
            load_cnt++;
            if(load_cnt>k){
                //printf("child exit : %d\n", k);
                shmdt(dom);
                shmdt(pc);
                shmdt(stat_cnt);
                shmdt(size);
                exit(k);
            }
        }
        //printf("pid number %d\n", getpid());
        //wget routine
        if(strstr(btok[load_cnt],option[0])!=NULL || strstr(btok[load_cnt],option[1])!=NULL ){
            (*pc)++;
            int2str(i2s,(*pc));
            if((pid=fork())==0){
                execlp("wget", "wget","-q", "--no-check-certificate","-O", i2s, btok[load_cnt],(char *)0);
            }
            else{
                wait(&status);
                if((fd=open(i2s,O_RDONLY))<0){
                    printf("%d>Error occurred!\n",(*pc));
                }
                else{
                    //Main domain
                    (*stat_cnt)++;
                    i=-1;
                    dtok[++i]=strpbrk(btok[load_cnt],".");
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
                    //title
                    read(fd,t_buff,10000);
                    ttok[0]=strstr(t_buff,"<title>");
                    ttok[1]=strstr(ttok[0]+7,"</title>");

                    memcpy(title,ttok[0]+7,ttok[1]-ttok[0]-7);
                    title[ttok[1]-ttok[0]-7]='\0';
                    close(fd);
                    t_size=(*size);
                    for(j=0;j<t_size;j++){
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
                    if(j==t_size){
                        len=(ttok[1]-ttok[0]-7);
                        memcpy(dom[j].title,title,len);
                        dom[j].title[len]='\0';
                        strcpy(dom[j].name,result);
                        dom[j].size=len;
                        (*size)++;
                    }

                    printf("%d>%s:%s\n", (*pc), dom[j].name,dom[j].title);
                }
            }
        }//wget routine end
        else if(strstr(btok[load_cnt],option[2])!=NULL){ //print routine
            (*pc)++;
            ttok[0]=strtok(btok[load_cnt]," ");
            ttok[0]=strtok(NULL," ");
            t_size=(*size);
            for(i=0;i<t_size;i++){
                if(strcmp(ttok[0],dom[i].name)==0){
                    printf("%d>%s:%s\n",(*pc),dom[i].name,dom[i].title);
                    break;
                }
            }
            if(i==t_size){
                printf("%d>Not Available\n", (*pc));
            }
        }
        else if(strstr(btok[load_cnt],option[3])!=NULL){ //stat routine
            (*pc)++;
            printf("%d>%d titles\n", (*pc), (*stat_cnt));
        }
        else if(strstr(btok[load_cnt],option[4])!=NULL){ //load routine
            ttok[0]=strtok(btok[load_cnt]," ");
            ttok[0]=strtok(NULL," ");
            printf("file name: %s\n", ttok[0]);
            pid=fork();
            if(pid==0){
                sign=0;
                stin=open(ttok[0],O_RDONLY);
            }
            else{
                wait(&status);
            }
        }
        else if(strstr(btok[load_cnt],option[5])!=NULL){ //quit routine
            shmdt(dom);
            shmdt(pc);
            shmdt(stat_cnt);
            shmdt(size);
            shmctl(i_shmid, IPC_RMID,0);
            shmctl(d_shmid, IPC_RMID,0);
            shmctl(s_shmid, IPC_RMID,0);
            shmctl(z_shmid, IPC_RMID,0);
            kill(0,SIGKILL);
            break;
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
