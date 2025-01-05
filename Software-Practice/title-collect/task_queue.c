#include "task_queue.h"

void TaskQueue_init(TaskQueue *queue) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void TaskQueue_destroy(TaskQueue *queue) {
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    while (queue->head != NULL) {
        Task *task = queue->head;
        queue->head = task->next;
        free(task);
    }
}

void TaskQueue_push(TaskQueue *queue, Task *task) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->tail != NULL) {
        queue->tail->next = task;
    } else {
        queue->head = task;
    }
    queue->tail = task;
    ++(queue->size);
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

Task *TaskQueue_pop(TaskQueue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    Task *task = queue->head;
    queue->head = task != NULL ? task->next : NULL;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    --(queue->size);
    pthread_mutex_unlock(&queue->mutex);

    return task;
}

void thread_pool_init(ThreadPool *pool, void *(*worker)(void *), size_t *count,  pthread_rwlock_t *rwlock, MemoryPool *bst_pool, BST **root) {
    TaskQueue_init(&pool->task_queue);
    pthread_mutex_init(&pool->count_lock, NULL);
    pthread_cond_init(&pool->all_done, NULL);
    pool->active_wget_count = 0;
    pool->done_wget_count = count;
    pool->rwlock = rwlock;
    pool->bst_pool = bst_pool;
    pool->root = root;

    for (int i = 0; i < MAX_THREAD_COUNT; ++i) {
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
}

void thread_pool_add_task(ThreadPool *pool, Task *task) {
    pthread_mutex_lock(&pool->count_lock);
    ++(pool->active_wget_count);
    pthread_mutex_unlock(&pool->count_lock);
    TaskQueue_push(&pool->task_queue, task);
}

void thread_pool_wait(ThreadPool *pool) {
    pthread_mutex_lock(&pool->count_lock);
    while (pool->active_wget_count > 0) {
        pthread_cond_wait(&pool->all_done, &pool->count_lock);
    }
    pthread_mutex_unlock(&pool->count_lock);
}

void thread_pool_destroy(ThreadPool *pool) {
    for (int i = 0; i < MAX_THREAD_COUNT; ++i) {
        Task *final_task = (Task *)malloc(sizeof(Task));
        final_task->final = true;
        final_task->next = NULL;
        thread_pool_add_task(pool, final_task);
    }
    for (int i = 0; i < MAX_THREAD_COUNT; ++i) {
        pthread_join(pool->threads[i], NULL);
    }
    TaskQueue_destroy(&pool->task_queue);
    pthread_mutex_destroy(&pool->count_lock);
    pthread_cond_destroy(&pool->all_done);
}