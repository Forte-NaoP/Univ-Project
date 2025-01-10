#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mystring.h"

typedef struct COMMAND {
    char *cmd;      // dynamically allocated
    char **argv;    // dynamically allocated
    int argc;
    char *input;    // dynamically allocated
    char *output;   // dynamically allocated
    bool append;
    int *rd_pipe;   // pointer variable
    int *wr_pipe;   // pointer variable
} COMMAND;

COMMAND *COMMAND_init();
void COMMAND_free(COMMAND *cmd);
bool parse_redirection(char *str, COMMAND *cmd);
bool parse_command(char *str, COMMAND *cmd);
COMMAND **split_commands(char *input, int *cmd_cnt);
void print_command(COMMAND *cmd);
#endif