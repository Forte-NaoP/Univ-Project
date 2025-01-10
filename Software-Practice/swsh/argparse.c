#include "argparse.h"

#define TYPE1 0
#define TYPE2 1
#define TYPE3 2
#define TYPE4 3

#define TYPE1_CNT 6
#define TYPE2_CNT 4
#define TYPE3_CNT 3
#define TYPE4_CNT 2

char cmd_list[][8] = {
    "ls", "man", "grep", "sort", "awk", "bc",
    "head", "tail", "cat", "cp",
    "mv", "rm", "cd",
    "pwd", "exit",
};

int cmd_cnt[] = {TYPE1_CNT, TYPE2_CNT, TYPE3_CNT, TYPE4_CNT};
char path[] = "./";

char *cmd_type1[TYPE1_CNT] = {"ls", "man", "grep", "sort", "awk", "bc"};
char *cmd_type2[TYPE2_CNT] = {"head", "tail", "cat", "cp"};
char *cmd_type3[TYPE3_CNT] = {"mv", "rm", "cd"};
char *cmd_type4[TYPE4_CNT] = {"pwd", "exit"};

COMMAND *COMMAND_init() {
    COMMAND *cmd = (COMMAND *)malloc(sizeof(COMMAND));
    cmd->cmd = NULL;
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = false;
    cmd->rd_pipe = NULL;
    cmd->wr_pipe = NULL;
    return cmd;
}

void COMMAND_free(COMMAND *cmd) {
    if (cmd->cmd != NULL) {
        free(cmd->cmd);
        cmd->cmd = NULL;
    }
    if (cmd->argv != NULL) {
        for (int i = 0; i < cmd->argc; ++i) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
        cmd->argv = NULL;
    }
    if (cmd->input != NULL) {
        free(cmd->input);
        cmd->input = NULL;
    }
    if (cmd->output != NULL) {
        free(cmd->output);
        cmd->output = NULL;
    }
}

COMMAND **split_commands(char *input, int *cmd_cnt) {
    char *cur = input, *saveptr = NULL;
    int cnt = 1;
    while (*cur != '\0') {
        if (*cur == '|') cnt += 1;
        cur += 1;
    }
    // printf("%s pipe: %d\n", input, cnt);
    COMMAND **cmds = (COMMAND **)malloc(sizeof(COMMAND *) * (cnt));
    cur = input;
    char *nxt = NULL;
    
    for (int i = 0; i < cnt; ++i) {
        cmds[i] = COMMAND_init();
        nxt = strstr(cur, " | ");
        if (nxt != NULL) {
            *nxt = '\0';
            nxt += 3;
        }
        if (!parse_redirection(cur, cmds[i])) {
            for (int j = 0; j <= i; ++j) {
                COMMAND_free(cmds[j]);
            }
            free(cmds);
            *cmd_cnt = -1;
            return NULL;
        }
        if (!parse_command(cur, cmds[i])) {
            for (int j = 0; j <= i; ++j) {
                COMMAND_free(cmds[j]);
            }
            free(cmds);
            *cmd_cnt = -1;
            return NULL;
        }
        cur = nxt;
    }
    *cmd_cnt = cnt;
    return cmds;
}

bool parse_redirection(char *str, COMMAND *cmd) {
    char *append_token = strstr(str, " >> ");
    char *out_token = strstr(str, " > ");
    char *in_token = strstr(str, " < ");
    
    if (append_token != NULL && (in_token != NULL || out_token != NULL)) {  
        // cannot have both append and input/output redirection
        COMMAND_free(cmd);
        return false;
    }

    if (out_token != NULL && in_token > out_token) {
        // cannot have input redirection after output redirection
        COMMAND_free(cmd);
        return false;
    }
    
    if (append_token != NULL) {
        if (strchr(append_token + 4, ' ') != NULL) {
            // cannot have more token after append redirection
            COMMAND_free(cmd);
            return false;
        }
        cmd->append = true;
        cmd->output = strdup(append_token + 4);
        *append_token = '\0';
        return true;
    }

    if (out_token != NULL) {
        if (strchr(out_token + 3, ' ') != NULL) {
            // cannot have more token after output redirection
            COMMAND_free(cmd);
            return false;
        }
        cmd->output = strdup(out_token + 3);
        *out_token = '\0';
    }

    if (in_token != NULL) {
        if (strchr(in_token + 3, ' ') != NULL) {
            // cannot have more token after input redirection
            COMMAND_free(cmd);
            return false;
        }
        cmd->input = strdup(in_token + 3);
        *in_token = '\0';
    }

    // if (cmd->input != NULL) {
    //     printf("in_redirect %s\n", cmd->input);
    // }
    // if (cmd->output != NULL) {
    //     printf("out_redirect %s\n", cmd->output);
    // }

    return true;
}

bool parse_command(char *str, COMMAND *cmd) {
    char *saveptr = NULL;
    // printf("cmd_str: %s---\n", str);
    char *cmd_name = strtok_r(str, " ", &saveptr);    
    int type = -1, idx = 0;
    for (int i = 0; i < 4 && type == -1; ++i) {
        for (int j = 0; j < cmd_cnt[i]; ++j, ++idx) {
            if (strcmp(cmd_name, cmd_list[idx]) == 0) {
                type = i;
                break;
            }
        }
    }

    if (type == -1) {
        if (strncmp(cmd_name, path, 2) == 0) {
            type = 1;
        } else {
            // command not found
            COMMAND_free(cmd);
            return false;
        }
    }

    if (type == TYPE4) {
        if (*saveptr != '\0') {
            // pwd and exit should not have any arguments
            COMMAND_free(cmd);
            return false;
        }
    }

    cmd->cmd = strdup(cmd_name);
    char *cur = saveptr;
    // printf("where: %s---\n", saveptr);
    
    if (*saveptr != '\0') cmd->argc = 1;
    while (*cur != '\0') {
        if (*cur == ' ') cmd->argc += 1;
        cur += 1;
    }
    // printf("argc: %d\n", cmd->argc);
    if (cmd->argc > 0) {
        cmd->argv = (char **)malloc(sizeof(char *) * (cmd->argc));
        int idx = 0;
        while (*saveptr != '\0') {
            char *arg = strtok_r(NULL, " ", &saveptr);
            // printf("where: %s---\n", arg);
            cmd->argv[idx] = strdup(arg);
            idx += 1;
        }
    }
    return true;
}

void print_command(COMMAND *cmd) {
    if (cmd->cmd != NULL) printf("cmd: %s\n", cmd->cmd);
    printf("argc: %d\n", cmd->argc);
    for (int i = 0; i < cmd->argc; ++i) {
        printf("argv[%d]: %s\n", i, cmd->argv[i]);
    }
    if (cmd->input != NULL) printf("input: %s\n", cmd->input);
    if (cmd->output != NULL) printf("output: %s\n", cmd->output);
    printf("append: %d\n", cmd->append);
    if (cmd->wr_pipe != NULL) printf("wr_pipe: %d\n", *cmd->wr_pipe);
    if (cmd->rd_pipe != NULL) printf("rd_pipe: %d\n", *cmd->rd_pipe);
    printf("------------------------\n");
}