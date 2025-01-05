#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#define MAX_THREAD_COUNT 4

#include <pthread.h>

#include "collect.h"
#include "pool.h"
#include "bst.h"

typedef struct Task {
    char url[MAX_DOMAIN_SIZE];
    char filename[16];
    bool final;
    struct Task *next;
} Task;

typedef struct TaskQueue {
    Task *head;
    Task *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t size;
} TaskQueue;

typedef struct ThreadPool {
    pthread_t threads[MAX_THREAD_COUNT];
    TaskQueue task_queue;
    pthread_mutex_t count_lock; // 작업 수 추적을 위한 뮤텍스
    pthread_cond_t all_done;   // 모든 작업 완료 대기 조건 변수
    size_t active_wget_count;  // 진행 중인 wget 작업 수
    size_t *done_wget_count;    // 완료된 wget 작업 수
    pthread_rwlock_t *rwlock;
    MemoryPool *bst_pool;
    BST **root;
} ThreadPool;

void TaskQueue_init(TaskQueue *queue);
void TaskQueue_destroy(TaskQueue *queue);
void TaskQueue_push(TaskQueue *queue, Task *task);
Task *TaskQueue_pop(TaskQueue *queue);

#endif