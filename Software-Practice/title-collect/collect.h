#ifndef COLLECT_H
#define COLLECT_H

#define MAX_BUFFER_SIZE 8192

#include <pthread.h>
#include <wait.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory_pool.h"
#include "bst.h"
#include "task_queue.h"

char *find_title(const char *name);
char *concat_string(const char *str1, ...);
char *get_domain_name(const char *url);

void wget(const char *url, const char *name, pthread_rwlock_t *rwlock, MemoryPool *bst_pool, BST **root, size_t *done_count);
void print(BST *root, char *domain, const char *name);
void status(size_t stat_count, const char *name);
void load();
void quit();

#endif