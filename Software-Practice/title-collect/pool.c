#include "pool.h"

void MemoryPool_init(MemoryPool *mp, size_t object_size, const char *name) {
#ifdef USE_SHARED_MEMORY
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        exit(EXIT_FAILURE);
    }

    size_t total_size = sizeof(MemoryPool) + POOL_SIZE * object_size + POOL_SIZE * sizeof(void *);
    if (ftruncate(fd, total_size) == -1) {
        exit(EXIT_FAILURE);
    }

    void *shared_mem = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_mem == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }
    close(fd);

    mp->pool = (uint8_t *)shared_mem + sizeof(MemoryPool);
    mp->free_list = (void **)((uint8_t *)mp->pool + POOL_SIZE * object_size);
#else
    mp->pool = malloc(POOL_SIZE * object_size);
    if (mp->pool == NULL) {
        exit(EXIT_FAILURE);
    }

    mp->free_list = (void **)malloc(POOL_SIZE * sizeof(void *));
    if (mp->free_list == NULL) {
        free(mp->pool);
        exit(EXIT_FAILURE);
    }
#endif
    for (int i = 0; i < POOL_SIZE; ++i) {
        mp->free_list[i] = (uint8_t *)mp->pool + i * object_size;
    }
    mp->free_count = POOL_SIZE;
}

void *MemoryPool_alloc(MemoryPool *mp) {
    if (mp->free_count == 0) {
        perror("free_count == 0");
        return NULL;
    }
    return mp->free_list[--(mp->free_count)];
}

void MemoryPool_free(MemoryPool *mp, void *ptr) {
    if (mp->free_count >= POOL_SIZE) {
        perror("free_count >= POOL_SIZE");
    }
    mp->free_list[mp->free_count++] = ptr;
}

void MemoryPool_destroy(MemoryPool *mp, size_t object_size) {
#ifdef USE_SHARED_MEMORY
    size_t total_size = sizeof(MemoryPool) + POOL_SIZE * object_size + POOL_SIZE * sizeof(void *);
    munmap(mp, total_size);
    shm_unlink("/bst_pool");
#else
    free(mp->pool);
    free(mp->free_list);
#endif
}