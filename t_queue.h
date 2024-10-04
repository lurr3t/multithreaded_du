/**
 * @defgroup t_queue_h t_queue
 *
 * @brief This datatype has the most common operations for a queue.
 *
 * Note, this is a queue that has the purpose of being a task queue for a thread pool to read
 * tasks from.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 1.0
 *
 * @{
 */

#ifndef T_QUEUE_H
#define T_QUEUE_H

#define CHAR_BUF 4096

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "string.h"

#include "list.h"
#include "error_handler.h"


/**
 * @brief                  A struct which is the structure of the task queue.
 *
 *                         Contains a list which will act as a queue with certain operations implemented.
 *                         Also has settings for the task queue as well as a thread pool.
 *
 * @elem task_q            A list which the queue is built upon.
 * @elem mutex             A variable for holding a mutex lock.
 * @elem cond              A condition variable.
 * @elem thread_amount     A amount of threads specified by the user.
 * @elem block_size        A variable for storing a block size.
 * @elem t_running         Amount of threads currently running.
 * @elem permission        A boolean to indicate if there was no permission to access a path.
 * @elem shutdown          A boolean that indicates for the threadpool when it's time to stop.
 *
 */
typedef struct task_queue {
    List *task_q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int thread_amount;
    blkcnt_t block_size;
    int t_running;
    bool permission;
    bool shutdown;
} Task_queue;

/**
 * @brief                 A struct which is the structure for a task
 *
 * @elem task_pointer     A function pointer, which points to a function that the threadpool will execute.
 */
typedef struct task {
    blkcnt_t (*task_pointer)(struct task *, Task_queue *);
    char *path;
} Task;


/**
 * @brief                Creates a task queue. Allocates memory and initializes it's values.
 *
 * @param thread_amount  The thread amount that will be stored in the queue.
 * @return               Returns a task queue that has been dynamically allocated.
 */
Task_queue *create_task_queue(int thread_amount);


/**
 * @brief                Creates a task, and allocates memory for it, also takes it's parameters as values.
 *
 * @param path           A path that will be stored in the task.
 * @param task_pointer   A function pointer to a function that the thread pool will execute.
 * @return               Returns a task that has been dynamically allocated.
 */
Task *create_task(char *path, void (*task_pointer)(struct task *, Task_queue *));


/**
 * @brief                Adds a task to the task queue.
 *
 * @param queue          The queue that the task will be added upon.
 * @param task           The task that will be added to the queue.
 */
void enqueue(Task_queue *queue, Task *task);


/**
 * @brief                Removes a task from the queue.
 *
 *                       Removes a task from the queue. Also copies the content of that task
 *                       to newly allocated memory and returns it. The memory of the task in the queue
 *                       is deallocated.
 *
 *                       NOTE! It's the user's responsibility to deallocate the returned value.
 *
 * @param queue          The queue that a task will be removed from.
 * @return               Returns a copy of the task on newly allocated memory. NULL if the queue is empty.
 */
Task *dequeue(Task_queue *queue);


/**
 * @brief                Checks if the queue is empty.
 *
 * @param queue          The queue that the check will be done upon.
 * @return               True if the queue is empty.
 */
bool queue_is_empty(Task_queue *queue);


/**
 * @brief                Deallocates the task's internal values, and the task itself.
 *
 * @param task           The task that will be deallocated.
 */
void kill_task(Task *task);


/**
 * @brief                Deallocates the queue and it's contents.
 * @param queue          The queue that will be deallocated.
 */
void destroy_queue(Task_queue *queue);


#endif //T_QUEUE_H

/**
 * @}
 */
