#include "collect.h"

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

void wget(const char *url, const char *name, pthread_rwlock_t *rwlock, MemoryPool *bst_pool, BST **root, size_t *done_count) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("wget", "wget","-q", "--no-check-certificate","-O", name, url, NULL);
    } else if (pid < 0) {
        raise(SIGINT);
    } else {
        waitpid(pid, NULL, 0);
        char *title = find_title(name);
        char *domain = get_domain_name(url);
        pthread_rwlock_wrlock(rwlock);
        BST_insert(bst_pool, root, domain, title);
        *done_count += 1;
        pthread_rwlock_unlock(rwlock);
        
        #ifdef DEBUG
        BST_print_inorder(*root);
        #endif

        char *output = concat_string(name, ">", domain, ":", title, "\n", NULL);
        write(1, output, strlen(output));
        free(domain);
        free(title);
        free(output);
    }
}

void print() {

}

void status() {

}

void load() {

}

void quit() {

}