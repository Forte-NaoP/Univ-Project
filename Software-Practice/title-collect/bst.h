#ifndef BST_H
#define BST_H

#include "mystring.h"
#include "pool.h"

#define MAX_DOMAIN_SIZE 128
#define MAX_TITLE_SIZE 1024

typedef struct BST {
    char title[MAX_TITLE_SIZE];
    char domain[MAX_DOMAIN_SIZE];
    struct BST *left;
    struct BST *right;
} BST;

BST* BST_new(MemoryPool *pool, char *domain, char *title);
BST* BST_search(BST *root, const char *domain);
void BST_insert(MemoryPool *pool, BST **root, char *domain, char *title);
void BST_print_inorder(BST *node);

#endif