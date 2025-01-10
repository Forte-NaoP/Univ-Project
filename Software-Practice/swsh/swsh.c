#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wait.h>

#include "mystring.h"
#include "argparse.h"

void chld_handler(int signo) {
    pid_t pid;
    int status;
    while((pid = waitpid(-1, &status, WUNTRACED)) > 0) {
        if(WIFSTOPPED(status)) {
            kill(-pid, SIGKILL);
        }
    }
}

int main() {
    char input[256];
    char output[256];
    size_t bytes;

    // struct sigaction sa;

    // sa.sa_handler = SIG_IGN;
    // sa.sa_flags = 0;
    // sigemptyset(&sa.sa_mask);
    // if (sigaction(SIGINT, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(EXIT_FAILURE);
    // }

    // sa.sa_handler = SIG_IGN;
    // if (sigaction(SIGTSTP, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(EXIT_FAILURE);
    // }

    // sa.sa_handler = chld_handler;
    // sa.sa_flags = SA_RESTART;
    // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(EXIT_FAILURE);
    // }

    while ((bytes = read(0, input, 256)) > 0) {
        input[bytes - 1] = '\0';
        int cmd_cnt = -1;
        COMMAND **cmds = split_commands(input, &cmd_cnt);
        if (cmd_cnt == -1) {
            printf("Error: Invalid command\n");
            continue;
        }
        
        for (int i = 0; i < cmd_cnt; ++i) {
            print_command(cmds[i]);
        }
        for (int i = 0; i < cmd_cnt; ++i) {
            COMMAND_free(cmds[i]);
        }
        // write(1, input, bytes);
    }
}