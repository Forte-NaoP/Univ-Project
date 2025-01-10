#include <stddef.h>

#define RE_IN 1
#define RE_OUT 2
#define RE_APEND 4
#define RE_PIPE 8
#define RE_PIPEOUT 16

extern char *parse[100];
extern char *f_name[2];
extern char src[100], dst[100];
extern int redirect[100];
extern int cmd_target[100];
extern int parse_index[100];
extern int idx;

extern int parse_input(char *input, int len);
extern int sub_parse(char *input, char **opt, int x);

void exec_func(char **input, int func, int x, int y);

int cd(char *name);
int my_pwd(char *name);
int my_exit(char *num);
