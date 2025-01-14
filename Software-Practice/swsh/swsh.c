#define _XOPEN_SOURCE 700

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
#include "commands/inline.h"

const char* custom_binary = "./commands/bin:";

void setup_redirection(COMMAND *cmd, int is_first, int is_last, int prev_pipe_fd, int pipe_fds[2]) {
    if (!is_first) {
        dup2(prev_pipe_fd, STDIN_FILENO);
        close(prev_pipe_fd);
    } else if (cmd->input) {
        int fd = open(cmd->input, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (!is_last) {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
    } else if (cmd->output) {
        int fd = open(cmd->output, O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC), 0666);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void restore_redirection(int stdin_fd, int stdout_fd) {
    dup2(stdin_fd, STDIN_FILENO);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdin_fd);
    close(stdout_fd);
}

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
    char *path = getenv("PATH");
    char *new_path = new_string(strlen(custom_binary) + strlen(path) + 1);
    strcat(new_path, custom_binary);
    strcat(new_path, path);
    setenv("PATH", new_path, 1);

    struct sigaction sa;
    
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = chld_handler;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    while ((bytes = read(0, input, 256)) > 0) {
        input[bytes - 1] = '\0';
        int cmd_cnt = -1;
        COMMAND **cmds = split_commands(input, &cmd_cnt);
        if (cmd_cnt == -1) {
            write(STDERR_FILENO, "swsh: Command not found\n", 24);
            continue;
        }

        int pipe_fds[2];
        int prev_pipe_fd = -1;

        for (int i = 0; i < cmd_cnt; ++i) {
            print_command(cmds[i]);
            COMMAND *cmd = cmds[i];
        
            if (i < cmd_cnt - 1) {
                if (pipe(pipe_fds) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            int stdin_fd = dup(STDIN_FILENO);
            int stdout_fd = dup(STDOUT_FILENO);

            if (cmd->need_fork) {
                pid_t pid = fork();
                if (pid == 0) {
                    setup_redirection(cmd, i == 0, i == cmd_cnt - 1, prev_pipe_fd, pipe_fds);

                    if (i < cmd_cnt - 1) {
                        close(pipe_fds[0]);
                        close(pipe_fds[1]);
                    }

                    if (prev_pipe_fd != -1) {
                        close(prev_pipe_fd);
                    }

                    execvp(cmd->cmd, cmd->argv);
                    exit(EXIT_FAILURE);
                }
            } else {
                if (strcmp(cmd->cmd, "cd") == 0) {
                    cd(cmd->argv[1]);
                } else if (strcmp(cmd->cmd, "pwd") == 0) {
                    char *wd = pwd();
                    write(STDOUT_FILENO, wd, strlen(wd));
                    free(wd);
                } else {
                    for (int j = 0; j < cmd_cnt; ++j) {
                        COMMAND_free(cmds[j]);
                    }
                    free(cmds);
                    shell_exit(cmd->argc > 1 ? atoi_32(cmd->argv[1]) : 0);
                }
            }

            int status;
            waitpid(-1, &status, 0);

            if (prev_pipe_fd != -1) {
                close(prev_pipe_fd);
            }
            if (i < cmd_cnt - 1) {
                close(pipe_fds[1]);
                prev_pipe_fd = pipe_fds[0];
            }

            restore_redirection(stdin_fd, stdout_fd);
        }

        for (int i = 0; i < cmd_cnt; ++i) {
            COMMAND_free(cmds[i]);
        }
        free(cmds);
    }
}