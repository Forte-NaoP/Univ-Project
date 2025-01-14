#ifndef INLINE_CMD_H
#define INLINE_CMD_H

#include "header_collection.h"

void cd(char *path);
char *pwd();
void shell_exit(int status);

#endif