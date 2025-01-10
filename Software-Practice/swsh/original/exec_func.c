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

void redirect_func(int *re_fd, int *ori_fd, int mod, int mod2)
{
    if(mod2==0){
        if(mod&RE_IN){
          re_fd[0]=open(f_name[0],O_RDONLY);
          if(re_fd[0]==-1){
              printf("swsh: No such file\n");
              return;
          }
          ori_fd[0]=dup(STDIN_FILENO);
          dup2(re_fd[0],STDIN_FILENO);
        }
        if(mod&RE_OUT){
            re_fd[1]=open(f_name[1],O_WRONLY|O_CREAT|O_TRUNC,0644);
            ori_fd[1]=dup(STDOUT_FILENO);
            dup2(re_fd[1],STDOUT_FILENO);
        }
        if(mod&RE_APEND){
            re_fd[1]=open(f_name[1],O_WRONLY|O_APPEND);
            ori_fd[1]=dup(STDOUT_FILENO);
            dup2(re_fd[1],STDOUT_FILENO);
        }
        if(mod&RE_PIPE){
            re_fd[0]=open(src,O_RDONLY);
            ori_fd[0]=dup(STDIN_FILENO);
            //printf("re_fd[0] : %d\n", re_fd[0]);
            dup2(re_fd[0],STDIN_FILENO);
        }
        if(mod&RE_PIPEOUT){
            re_fd[1]=open(dst,O_WRONLY|O_CREAT|O_TRUNC,0644);
            ori_fd[1]=dup(STDOUT_FILENO);
            //printf("re_fd[1] : %d\n", re_fd[1]);
            dup2(re_fd[1],STDOUT_FILENO);
        }
    }
    else if(mod2==1){
        if(mod&RE_IN){
            close(re_fd[0]);
            dup2(ori_fd[0],STDIN_FILENO);
        }
        if(mod&RE_OUT||mod&RE_APEND){
            close(re_fd[1]);
            dup2(ori_fd[1],STDOUT_FILENO);
        }
        if(mod&RE_PIPE){
            close(re_fd[0]);
            dup2(ori_fd[0],STDIN_FILENO);
        }
        if(mod&RE_PIPEOUT){
            close(re_fd[1]);
            dup2(ori_fd[1],STDOUT_FILENO);
        }
    }
}

void exec_func(char **input, int func, int x, int y)
{
    pid_t pid;
    int status;
    int re_fd[2], re_fout[2];
    int ori_fd[2], ori_out[2];

    if((x==0 || x==y-1) && redirect[x]!=0) redirect_func(re_fd, ori_fd, redirect[x], 0);
    if(x!=0) redirect_func(re_fd, ori_fd, RE_PIPE,0);
    if(x<y-1) redirect_func(re_fout, ori_out, RE_PIPEOUT,0);

    if(func>11){
        if(func==12){
            cd(input[1]);
        }
        if(func==13){
            my_pwd(input[0]);
        }
        if(func==14){
            my_exit(input[1]);
        }
    }
    else{
        pid=fork();
        if(pid==0){
            setpgid(0,getpid());
            execvp(input[0],input);
            printf("%s\n", strerror(errno));
        }
        else{
            pause();
            /*waitpid(-1,&status,  WUNTRACED);
            if(WIFSTOPPED(status)){
                printf("true\n" );
                kill(-pid, SIGKILL);
            }*/
        }
    }
    if(redirect[x]!=0) redirect_func(re_fd, ori_fd, redirect[x], 1);
    if(x<y-1) redirect_func(re_fout, ori_out, RE_PIPEOUT,1);
    if(x!=0) redirect_func(re_fd, ori_fd, RE_PIPE,1);

}
