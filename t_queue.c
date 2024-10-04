/**
 * @brief This datatype has the most common operations for a queue.
 *
 * Note, this is a queue that has the purpose of being a task queue for a thread pool to read
 * tasks from.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 1.0
 *
 */

#include "t_queue.h"

Task_queue *create_task_queue(int thread_amount) {
    Task_queue *q = malloc(sizeof(Task_queue));
    error_handler_null(q, NULL, "queue couldn't allocate memory", true);
    q->task_q = list_create();
    q->thread_amount = thread_amount;
    q->block_size = 0;
    q->t_running = 0;
    q->shutdown = false;
    q->permission = true;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}

Task *create_task(char *path, void (*task_pointer)(struct task *, Task_queue *)) {
    Task *task = malloc(sizeof(Task));
    error_handler_null(task, NULL, "task couldn't allocate memory", true);
    task->path = path;
    task->task_pointer = (blkcnt_t (*)(struct task *, Task_queue *)) (void (*)(void)) task_pointer;
    return task;
}


void enqueue(Task_queue *queue, Task *task) {
    List *list = queue->task_q;
    ListPos first_pos = list_prev(list_first(list));
    list_insert(first_pos, task);
}


/**
 * Removes a task from the task queue. Also creates a copy of that task, and allocates new memory for it.
 * The task that was originally in the queue is deallocated. The function returns the copied task.
 *
 */
Task *dequeue(Task_queue *queue) {
    Task *copy_task = NULL;
    if (!queue_is_empty(queue)) {
        ListPos task_pos = list_prev(list_end(queue->task_q));
        Task *task = list_inspect(task_pos);
        copy_task = malloc(sizeof(Task));
        error_handler_null(copy_task, NULL, "copy_task couldn't allocate memory", true);
        *copy_task = *task;
        //frees the old task
        list_remove(task_pos);
    }
    return copy_task;
}

bool queue_is_empty(Task_queue *queue) {
    return list_is_empty(queue->task_q);
}

void destroy_queue(Task_queue *queue) {
    while (!queue_is_empty(queue)) {
        kill_task(dequeue(queue));
    }
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
    free(queue->task_q);
    free(queue);
}

void kill_task(Task *task) {
    if (task != NULL) {
        if (task->path != NULL) {
            free(task->path);
        }
        free(task);
    }
}