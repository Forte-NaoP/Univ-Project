#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "swsh_header.h"

#define CMD_NUM 15

char cmd_table[CMD_NUM][10]={"ls","man","grep","sort","awk","bc",
                        "head", "tail", "cat", "cp", "mv",
                        "rm", "cd", "pwd","exit"};

int parse_input(char *input, int len)
{
    int i,j,k=0;

    for(i=0;i<100;i++){
        redirect[i]=0;
        cmd_target[i]=-1;
    }
    i=0;
    parse[i]=input;
    //printf("in parse: %s\n", parse[i]);
    i++;
    parse[i]=strpbrk(input, "|");
    while(parse[i]!=NULL){
        i++;
        parse[i]=strpbrk(parse[i-1]+1, "|");
    }
    //printf("%d\n", i);

    for(j=1;j<i;j++){
        *(parse[j]-1)='\0';
        *(parse[j]+1)='\0';
        parse[j]+=2;
    }
    //for(j=0;j<i;j++) printf("in parse: %s\n", parse[j]);
    //for(j=0;j<i;j++) printf("redi: %d\n", redirect[j]);

    for(j=0;j<i;j++){
        for(k=0;k<CMD_NUM;k++){
            if(strinstr(parse[j],cmd_table[k])!=NULL){
                cmd_target[j]=k;
                //printf("table : %d\n", k);
                break;
            }
        }
        if(k==CMD_NUM){
            if(strstr(parse[j],"./")==NULL) return -1;
        }
    }
    return i;
}

int sub_parse(char *input, char **opt, int x)
{
    int i,k;
    int len; //strlen(input);
    char *c, *d;

    opt[0]=input;
    if((c=strpbrk(input,"<>"))==NULL) len=strlen(input);
    else{
        len=(c-input-1);
        *(c-1)='\0';
    }

    for(k=1,i=0;i<len;i++){
        if(input[i]==' '){
              input[i]='\0';
              opt[k++]=((&input[i])+1);
        }
    }
    opt[k]=NULL;

    //for(i=0;i<k;i++) printf("%s\n", opt[i]);

    if(c!=NULL){
        if(c[0]=='<'){
            f_name[0]=c+2;
            redirect[x]|=RE_IN;
            if((d=strpbrk(c,">"))!=NULL){
                *(d-1)='\0';
                if(d[1]=='>'){
                    f_name[1]=d+3;
                    redirect[x]|=RE_APEND;
                }
                else {
                    f_name[1]=d+2;
                    redirect[x]|=RE_OUT;
                }
            }
        }
        else if(c[0]=='>'){
            if(c[1]=='>'){
                f_name[1]=c+3;
                redirect[x]|=RE_APEND;
            }
            else {
                f_name[1]=c+2;
                redirect[x]|=RE_OUT;
            }
        }
    }

    return 0;
}
