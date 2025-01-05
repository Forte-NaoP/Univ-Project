#include "taskqueue.h"

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