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
#include "pool.h"
#include "bst.h"

#define SEM_NAME "/collect_sem"
#define MAX_SEM_COUNT 1
#define HALF_BUFFER_SIZE 4096
#define MAX_BUFFER_SIZE 8192
#define MAX_THREAD_COUNT 8
#define DEBUG

void initialize();
void finalize();
void wget();
void print();
void status();
void load();
void quit();
void collect();
char *get_domain_name(const char *url);

char commands[][8] = {"print", "stat", "load", "quit"};
char protocol[][16] = {"http://", "https://"};

sem_t *sem;
int current_counter;
int stat_count;
pthread_rwlock_t rwlock;

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

MemoryPool bst_pool;
BST *root;

void initialize() {
    MemoryPool_init(&bst_pool, sizeof(BST), "/bst_pool");
    pthread_rwlock_init(&rwlock, NULL);
    setup_signal_handler();
}

void finalize() {
    MemoryPool_destroy(&bst_pool, sizeof(BST));
    pthread_rwlock_destroy(&rwlock);
}

char *find_title(const char *name) {
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        return NULL;
    } else {
        char buffer[MAX_BUFFER_SIZE + 1];
        ssize_t bytes = read(fd, buffer, MAX_BUFFER_SIZE);
        char *title_st, *title_ed;
        buffer[bytes] = '\0';
        title_st = strstr(buffer, "<title>");
        if (title_st == NULL) {
            return NULL;
        }
        title_st += 7;

        title_ed = strstr(title_st, "</title>");
        if (title_ed == NULL) {
            return NULL;
        }
        *title_ed = '\0';

        char *title = strdup(title_st);
        #ifdef DEBUG
        printf("find_title: %s\n", title);
        #endif
        close(fd);
        return title;
    }
}

char *concat_string(const char *str1, ...) {
    va_list args;
    size_t total_len = strlen(str1);
    char *str;
    va_start(args, str1);
    while ((str = va_arg(args, char *)) != NULL) {
        total_len += strlen(str);
    }
    va_end(args);

    char *result = new_string(total_len + 1);
    strcpy(result, str1);

    va_start(args, str1);
    while ((str = va_arg(args, char *)) != NULL) {
        strcat(result, str);
    }
    va_end(args);
    #ifdef DEBUG
    printf("concat_string: %s\n", result);
    #endif
    return result;
}

void wget(const char *url, const char *name) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("wget", "wget","-q", "--no-check-certificate","-O", name, url, NULL);
    } else if (pid < 0) {
        raise(SIGINT);
    } else {
        waitpid(pid, NULL, 0);
        char *title = find_title(name);
        char *domain = get_domain_name(url);
        pthread_rwlock_wrlock(&rwlock);
        BST_insert(&bst_pool, &root, domain, title);
        pthread_rwlock_unlock(&rwlock);
        #ifdef DEBUG
        BST_print_inorder(root);
        #endif
        char *output = concat_string(name, ">", domain, ":", title, "\n", NULL);
        write(1, output, strlen(output));
        free(domain);
        free(title);
        free(output);
    }
}

char *get_domain_name(const char *url) {
    char copy[strlen(url) + 1];
    strcpy(copy, url);

    char *saveptr = NULL;
    char *domain_st = strstr(copy, "://");
    if (domain_st == NULL) {
        return NULL;
    }
    domain_st += 3;

    char *domain_ed = strchr(domain_st, '/');
    if (domain_ed != NULL) {
        *domain_ed = '\0';
    }

    char *tld = strrchr(domain_st, '.');
    if (tld == NULL || tld == domain_st) {
        return NULL;
    }

    int ld_count = (strcmp(tld, ".kr") == 0) ? 2 : 1;
    char *ld = tld;
    while (ld_count > 0 && ld != domain_st - 1) {
        --ld;
        if (*ld == '.') {
            --ld_count;
        }
    }

    char *domain = strdup(ld + 1);
    #ifdef DEBUG
    printf("get_domain: %s\n", domain);
    #endif
    return domain;
}

void print() {

}

void status() {

}

void load() {

}

void quit() {

}

void collect() {
    initialize();
    char buffer[MAX_BUFFER_SIZE + 1];
    char *lines[2048] = {NULL};
    size_t line_count;
    

    while (true) {
        line_count = 0;
        ssize_t bytes = read(0, buffer, MAX_BUFFER_SIZE);
        buffer[bytes] = '\0';
        char *saveptr = NULL;
        char *token = strtok_r(buffer, "\n", &saveptr); 
        lines[line_count++] = token;
        while ((token = strtok_r(NULL, "\n", &saveptr)) != NULL) {
            lines[line_count++] = token;
        }

        for (size_t i = 0; i < line_count; ++i) {
            if (strstr(lines[i], "://") != NULL) {
                pthread_rwlock_wrlock(&rwlock);
                current_counter += 1;
                char name[16];
                int2str(name, current_counter);
                pthread_rwlock_unlock(&rwlock);
                /*
                    TODO: Implement thread pool here
                */
                wget(lines[i], name);
            }
        }
    }
    finalize();
}

int main(int argc, char* argv[]) {
    collect();
    return 0;
}
