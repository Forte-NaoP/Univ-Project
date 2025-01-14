#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "mystring.h"

#define TYPE1 0
#define TYPE2 1
#define TYPE3 2
#define TYPE4 3

#define TYPE1_CNT 6
#define TYPE2_CNT 4
#define TYPE3_CNT 3
#define TYPE4_CNT 2

typedef struct COMMAND {
    char *cmd;      // dynamically allocated
    char **argv;    // dynamically allocated
    int argc;
    char *input;    // dynamically allocated
    char *output;   // dynamically allocated
    bool append;
    bool need_fork;
} COMMAND;

COMMAND *COMMAND_init();
void COMMAND_free(COMMAND *cmd);
bool parse_redirection(char *str, COMMAND *cmd);
bool parse_command(char *str, COMMAND *cmd);
COMMAND **split_commands(char *input, int *cmd_cnt);
void print_command(COMMAND *cmd);
#endif