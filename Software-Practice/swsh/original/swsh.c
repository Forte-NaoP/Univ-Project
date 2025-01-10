#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "mystring.h"
#include "swsh_header.h"

char *parse[100]={};
char *f_name[2];
char src[100], dst[100];
int redirect[100]={};
int cmd_target[100]={};
int parse_index[100]={};
int idx=0;

char *pwd (int full)
{
    static char buffer[1024];
    char *ptr;
    if (getcwd(buffer, 1024) != buffer)
        return NULL;

    if (full)
        return buffer;

    if (strcmp("/", buffer) == 0)
        return buffer;

    ptr = strrchr(buffer, '/');
    return ptr+1;
}

void print_prompt()
{
    int ret;
    char *ptr = pwd(0);
    char head[200];

    if (!ptr)
        exit(1);
    if (strlen(ptr) > 190)
        exit(1);

    strcpy(head, "swsh:");
    strcat(head, ptr);
    strcat(head, "> ");

    ret = write(2, head, strlen(head));
    if (ret <= 0)
        exit(1);
}

void chld_handler()
{
    pid_t pid;
    int status;
    while(pid=waitpid(-1, &status, WUNTRACED)>0){
        if(WIFSTOPPED(status)){
            kill(-pid, SIGKILL);
        }
    }
}

int main(void)
{
    int ret;
    int i, fd;
    int cnt;

    char input[240];
    char *opt[100][100];
    char tmp[1000]="";
    char env[1000]="PATH=";
    char *c;

    pid_t pid;
    signal(SIGINT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGCHLD, chld_handler);

    c=getenv("PATH");
    getcwd(tmp,1000);
    strcat(tmp,"/bin");
    strcat(env,tmp);
    strcat(env,":");
    strcat(env,c);
    putenv(env);

    getcwd(src,100);
    getcwd(dst,100);
    strcat(src,"/.rd");
    strcat(dst,"/.rc");

    fd=open(src,O_WRONLY|O_CREAT,0644);
    close(fd);
    fd=open(dst,O_WRONLY|O_CREAT,0644);
    close(fd);

    printf("swsh pid: %d\n", getpid());
    while (1) {
        print_prompt();

        ret = read(0, input, 200);
        if (ret < 0)
            exit(1);
        else if (ret == 0) {
            write(2, "exit\n", 5);
            break;
        }
        write(2, input, ret);
        input[--ret]='\0';
        if((cnt=parse_input(input,ret))==-1){
            write(2,"swsh: Command not found\n", 25);
        }
        else{
            for(i=0;i<cnt;i++){
                sub_parse(parse[i],opt[i],i);
            }
            for(i=0;i<cnt;i++){
                exec_func(opt[i],cmd_target[i],i, cnt);
                pid=fork();
                if(pid==0){
                    execl("/bin/cp", "cp", dst, src, NULL);
                }
                else{
                    pause();
                }
            }
        }

        /*memset(input,0,240);
        memset(parse,0,sizeof(parse));
        memset(redirect,0,sizeof(redirect));
        memset(cmd_target,0,sizeof(cmd_target));
        memset(parse_index,0,sizeof(parse_index));
        memset(opt,0,sizeof(opt));*/
        memset(f_name,0,sizeof(f_name));
        idx=0;

    }

    return 0;
}
