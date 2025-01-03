#ifndef POOL_H
#define POOL_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define POOL_SIZE 1024
// #define USE_SHARED_MEMORY

typedef struct MemoryPool {
    void *pool;
    void **free_list;
    int free_count;
} MemoryPool;

void MemoryPool_init(MemoryPool *mp, size_t object_size, const char *name);
void *MemoryPool_alloc(MemoryPool *mp);
void MemoryPool_free(MemoryPool *mp, void *ptr);
void MemoryPool_destroy(MemoryPool *mp, size_t object_size);

#endif