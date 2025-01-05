//-----------------------------------------------------------
// 2016313621 BAE JUN HWI
//
// SWE2007: Software Experiment II (Fall 2017)
//
// Skeleton code for PA //2
// September 27, 2017
//
// Jong-Won Park
// Embedded Software Laboratory
// Sungkyunkwan University
//
//-----------------------------------------------------------
#define _XOPEN_SOURCE 700

#include <wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "mystring.h"
#include "memory_pool.h"
#include "bst.h"
#include "collect.h"
#include "task_queue.h"

#define SEM_NAME "/collect_sem"
#define MAX_SEM_COUNT 1

void initialize();
void finalize();
void collect();

char commands[][8] = {"print", "stat", "load", "quit"};
char protocol[][16] = {"http://", "https://"};

sem_t *sem;
size_t current_counter;
size_t stat_count;
MemoryPool bst_pool;
BST *root;
pthread_rwlock_t rwlock;
ThreadPool thread_pool;

typedef struct Backup {
    char **lines;
    size_t line_count;
    int input_fd;
    struct Backup *next;
} Backup;

typedef struct BackupStack {
    Backup *top;
} BackupStack;

BackupStack backup_stack;

void backup(BackupStack *stack, char **lines, size_t size, int input_fd) {
    Backup *backup = (Backup *)malloc(sizeof(Backup));  
    backup->lines = (char **)malloc(sizeof(char *) * size);
    for (size_t i = 0; i < size; ++i) {
        backup->lines[i] = strdup(lines[i]);
    }
    backup->line_count = size;
    backup->input_fd = input_fd;

    backup->next = stack->top;
    stack->top = backup;
}

void restore(BackupStack *stack, char **lines, size_t *size, int *input_fd) {
    Backup *backup = stack->top;
    stack->top = backup->next;

    for (size_t i = 0; i < backup->line_count; ++i) {
        strcpy(lines[i], backup->lines[i]);
    }

    *size = backup->line_count;
    *input_fd = backup->input_fd;

    for (size_t i = 0; i < backup->line_count; ++i) {
        free(backup->lines[i]);
    }

    free(backup->lines);
    free(backup);
}

struct sigaction old_action;
void sigint_handler(int signo) {
    if (signo == SIGINT) {
        finalize();
        sigaction(SIGINT, &old_action, NULL);
        raise(SIGINT);
    }
}

void setup_signal_handler() {
    struct sigaction new_action;

    new_action.sa_handler = sigint_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if (sigaction(SIGINT, &new_action, &old_action) < 0) {
        exit(EXIT_FAILURE);
    }
}

void *thread_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (true) {
        Task *task = TaskQueue_pop(&pool->task_queue);
        if (task->final) {
            free(task);
            break;
        }
        
        wget(task->url, task->filename, pool->rwlock, pool->bst_pool, pool->root, pool->done_wget_count);
        
        pthread_mutex_lock(&pool->count_lock);
        --(pool->active_wget_count);
        if (pool->active_wget_count == 0) {
            pthread_cond_signal(&pool->all_done);
        }
        pthread_mutex_unlock(&pool->count_lock);
        
        free(task);
    }
    return NULL;
}

void initialize() {
    thread_pool_init(&thread_pool, thread_worker, &stat_count, &rwlock, &bst_pool, &root);
    MemoryPool_init(&bst_pool, sizeof(BST), "/bst_pool");
    pthread_rwlock_init(&rwlock, NULL);
    backup_stack.top = NULL;
    setup_signal_handler();
}

void finalize() {
    thread_pool_destroy(&thread_pool);
    MemoryPool_destroy(&bst_pool, sizeof(BST));
    pthread_rwlock_destroy(&rwlock);
}

void collect() {
    initialize();
    char buffer[MAX_BUFFER_SIZE + 1];
    char *lines[2048] = {NULL};
    size_t line_count = 0;
    int input = STDIN_FILENO;

    while (true) {
        ssize_t bytes = read(input, buffer, MAX_BUFFER_SIZE);
        if (bytes == 0) {
            if (input == STDIN_FILENO && backup_stack.top == NULL && line_count == 0) {                
                finalize();
                return;
            }
            if (input != STDIN_FILENO) close(input);
            if (backup_stack.top == NULL) {
                input = STDIN_FILENO;
            } else {
                restore(&backup_stack, lines, &line_count, &input);
            }
            if (line_count == 0) continue;
        }
        if (bytes > 0 ) {
            buffer[bytes] = '\0';
            char *saveptr = NULL;
            char *token = strtok_r(buffer, "\n", &saveptr); 
            lines[line_count++] = token;
            while ((token = strtok_r(NULL, "\n", &saveptr)) != NULL) {
                lines[line_count++] = token;
            }
        }
        for (size_t i = 0; line_count > 0; ++i, --line_count) {
            char name[16];
            current_counter += 1;
            int2str(name, current_counter);
            if (strstr(lines[i], "://") != NULL) {
                Task *task = (Task *)malloc(sizeof(Task));
                strcpy(task->url, lines[i]);
                strcpy(task->filename, name);
                task->next = NULL;
                thread_pool_add_task(&thread_pool, task);
            } else if (strstr(lines[i], "print") != NULL) {
                thread_pool_wait(&thread_pool);
                print(root, lines[i] + 6, name);
            } else if (strstr(lines[i], "stat") != NULL) {
                thread_pool_wait(&thread_pool);
                status(stat_count, name);
            } else if (strstr(lines[i], "load") != NULL) {
                backup(&backup_stack, lines + i + 1, line_count - 1, input);
                input = open(lines[i] + 5, O_RDONLY);
                line_count = 0;
                break;
            } else if (strstr(lines[i], "quit") != NULL) {
                finalize();
                return;
            }
        }
    }
    finalize();
}

int main(int argc, char* argv[]) {
    collect();
    return 0;
}
