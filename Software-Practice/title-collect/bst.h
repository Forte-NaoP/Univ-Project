#ifndef BST_H
#define BST_H

#include "mystring.h"
#include "pool.h"

typedef struct BST {
    char title[1024];
    char domain[128];
    struct BST *left;
    struct BST *right;
} BST;

BST* BST_new(MemoryPool *pool, char *domain, char *title);
BST* BST_search(BST *root, const char *domain);
void BST_insert(MemoryPool *pool, BST **root, char *domain, char *title);
void BST_print_inorder(BST *node);

#endif