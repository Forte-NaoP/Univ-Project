#include "mystring.h"
#include "bst.h"
#include "pool.h"

BST* BST_new(MemoryPool *pool, char *domain, char *title) {
    BST *node = (BST *)MemoryPool_alloc(pool);
    if (node == NULL) {
        exit(EXIT_FAILURE);
    }
    strcpy(node->domain, domain);
    strcpy(node->title, title);
    node->left = NULL;
    node->right = NULL;
    return node;
}

BST* BST_search(BST *root, const char *domain) {
    BST *cur = root;
    BST *prev = NULL;
    int cmp;
    while (cur != NULL) {
        cmp = strcmp(domain, cur->domain);
        if (cmp < 0) {
            prev = cur;
            cur = cur->left;
        } else if (cmp > 0) {
            prev = cur;
            
            cur = cur->right;
        } else {
            return cur;
        }
    }
    return prev;
}

void BST_insert(MemoryPool *pool, BST **root, char *domain, char *title) {
    if (*root == NULL) {
        *root = BST_new(pool, domain, title);
        return;
    }

    BST *prev = BST_search(*root, domain);
    int cmp = strcmp(domain, prev->domain);
    if (cmp < 0) {
        prev->left = BST_new(pool, domain, title);
    } else if (cmp > 0) {
        prev->right = BST_new(pool, domain, title);
    } else {
        if (strlen(title) > strlen(prev->title)) {
            strcpy(prev->title, title);
        }
    }
}

void BST_print_inorder(BST *node) {
    if (node == NULL) return;
    BST_print_inorder(node->left);
    printf("Domain: %s, Title: %s\n", node->domain, node->title);
    BST_print_inorder(node->right);
}