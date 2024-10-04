/**
 *
 * @brief A program responsible for simulating some functionality of [du] in a unix shell.
 * Apart from [du] this program can also calculate the size by doing it with multithreading.
 *
 * If the multithreading option is chosen, the task will be done with a threadpool implementation.
 *
 * The program has the same output as [du] and takes the following arguments and flags.
 * Leave the -j flag and [thread amount] out if you only want to calculate the size with one thread recursively.
 *
 * [-j] [Thread amount]                        The flag combined with an integer, to specify the amount of
 *                                             threads to be used.
 *
 * [path] or [paths...]                        One or more paths. The program will calculate the entire depth
 *                                             of the file tree, where the root is the path.
 *
 * NOTE! The only argument that is required, is at least one path. Leave the -j flag out, and one thread will do
 * the task.
 *
 * @author  Ludwig Fallström
 * @since   2021-10-14
 * @version 2.0
 *
 * @{
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include "string.h"
#include <dirent.h>
#include "list.h"
#include "t_queue.h"
#include "error_handler.h"

void start_options_and_run(Task_queue *t_queue, List *targets);
void make_path(char *new_path, const char *name, const char *absolute_path);
void flag_options(int argc, char *argv[], int *thread_amount);
List *path_name_parser(int argc, char *const *argv);
blkcnt_t get_block_size(char *absolute_path, bool *permission);
void add_task(Task_queue *t_queue, Task *task);
blkcnt_t get_block_size_mult(Task *task, Task_queue *queue);
void *run_thread(Task_queue *t_queue);
void run_task(Task_queue *t_queue, Task *task);
void run_mult_thread(Task_queue *t_queue, char *start_path);
blkcnt_t shutdown_threads(Task *task, Task_queue *queue);
blkcnt_t get_size_of_dir(Task *task, void *queue_or_permission,
                         const char *absolute_path, struct stat *absolute_path_buf, DIR *dir, bool multithread);
void kill_task_initializer(Task_queue *t_queue, blkcnt_t temp_block_size, bool queue_empty, int t_running);



int main(int argc, char **argv) {
    int thread_amount = 1;
    flag_options(argc, argv, &thread_amount);
    List *path_names = path_name_parser(argc, argv);
    Task_queue *t_queue = create_task_queue(thread_amount);

    //the function that starts everything
    start_options_and_run(t_queue, path_names);

    bool permission = t_queue->permission;
    list_destroy(path_names);
    destroy_queue(t_queue);
    if (permission) { exit(EXIT_SUCCESS); }
    exit(EXIT_FAILURE);
}


/**
 * @brief                                      Chooses if the program will run with multiple threads or not.
 *
 *                                             Also goes through the list of paths, and makes sure to
 *                                             run each one of them.
 *
 * @param t_queue                              The task queue containing.
 * @param targets                              The list of paths that the size will be calculated upon.
 */
void start_options_and_run(Task_queue *t_queue, List *targets) {
    ListPos current_pos = list_first(targets);
    //loops through the target list
    while (!list_pos_equal(current_pos, list_end(targets))) {
        char *path = (char *)list_inspect(current_pos);

        //options if the program will be multithreaded, or done recursively.
        if (t_queue->thread_amount > 1) {
            run_mult_thread(t_queue, path);
        } else {
            t_queue->block_size = get_block_size(path, &t_queue->permission);
        }
        printf("%ld\t%s\n", t_queue->block_size, path);

        //nulls the variables that has been changed
        t_queue->block_size = 0;
        t_queue->t_running = 0;
        t_queue->shutdown = false;

        //clears the queue
        while (!queue_is_empty(t_queue)) {
            kill_task(dequeue(t_queue));
        }
        current_pos = list_next(current_pos);
    }
}


/**
 * @brief                                      The main algorithm for calculating the size recursively.
 *
 *                                             Calculates the entire depth of the absolute_path parameter.
 *                                             If an error occurred the permission is changed to false.
 *
 *                                             NOTE! That this function is in a recursive call chain
 *
 * @param absolute_paths                       The path that will be the root of the file tree search.
 * @param permission                           A variable that will indicate if an error occurred. False if so.
 * @return                                     Returns the size of the entire file tree, originating in absolute_path.
 */
blkcnt_t get_block_size(char *absolute_path, bool *permission) {
    blkcnt_t block_size = 0;
    struct stat absolute_path_buf;
    int check = lstat(absolute_path, &absolute_path_buf);
    if (check < 0) { return 0; }

    //if dir
    if (S_ISDIR(absolute_path_buf.st_mode)) {
        //opens dir
        DIR *dir = opendir(absolute_path);
        if (dir == NULL) {
            fprintf(stderr, "mdu: cannot read directory '%s': Permission denied\n", absolute_path);

            //sets permission to false if the directory is not readable
            *permission = false;
            return absolute_path_buf.st_blocks;
        }

        block_size += get_size_of_dir(NULL, permission, absolute_path, &absolute_path_buf, dir, false);
    } else {
        block_size += absolute_path_buf.st_blocks;
    }
    return block_size;
}


/**
 * @brief                                      The main algorithm for calculating the size with multiple threads.
 *
 *                                             Calculates the size of the path.
 *                                             If it's a file, the file size will be returned.
 *                                             If it's a directory, the size of it's contents will be
 *                                             returned. And the paths to directories will be added to
 *                                             the task queue.
 *                                             If an error occurred the permission is changed to false.
 *
 * @param task                                 A task containing a path that the size will be calculated upon.
 * @param queue                                A task queue, containing settings, and for being added toı
 * @return                                     Returns the size of the path contained in the task.
 */
blkcnt_t get_block_size_mult(Task *task, Task_queue *queue) {
    blkcnt_t block_size = 0;
    char *absolute_path = task->path;
    struct stat absolute_path_buf;
    int check = lstat(absolute_path, &absolute_path_buf);
    if (check < 0) {
        return 0;
    }

    //if dir
    if (S_ISDIR(absolute_path_buf.st_mode)) {
        //opens dir
        DIR *dir = opendir(absolute_path);
        if (dir == NULL) {
            fprintf(stderr, "mdu: cannot read directory '%s': Permission denied\n", absolute_path);
            pthread_mutex_lock(&queue->mutex);
            queue->permission = false;
            pthread_mutex_unlock(&queue->mutex);
            return absolute_path_buf.st_blocks;
        }
        block_size = get_size_of_dir(task, queue, absolute_path, &absolute_path_buf, dir, true);
    } else {
        block_size += absolute_path_buf.st_blocks;
    }
    return block_size;
}


/**
 * @brief                                      The main algorithm for going through a directory and calculating it's
 *                                             contents total size.
 *
 *                                             In multithreading mode, tasks will be added to the task queue, if
 *                                             new directories inside is found. In recursive mode, the get_block_size
 *                                             function will be called with the new path to a directory.
 *
 * @param task                                 A task containing a function pointer, which will be used to create
 *                                             a new task. Set to NUll if the program should be used with one thread.
 *
 * @param queue_or_permission                  Takes either a pointer to a queue, or a boolean. A queue if it is
 *                                             multithreaded, Otherwise a boolean which holds an indication if
 *                                             there was an error.
 *
 * @param absolute_path                        The path to the directory. Will be used when creating new path names.
 * @param absolute_path_buf                    A struct stat which holds information of the absolute_path.
 * @param dir                                  A pointer to an opened directory.
 * @param multithread                          Set to true, if it should be used with multithreading.
 * @return                                     Returns the size of a directory.
 */
blkcnt_t get_size_of_dir(Task *task, void *queue_or_permission,
                         const char *absolute_path, struct stat *absolute_path_buf, DIR *dir, bool multithread) {
    blkcnt_t block_size = 0;
    struct dirent *dir_struct;
    char *new_absolute_path;
    //if directory has content
    while ((dir_struct = readdir(dir)) != NULL) {
        //allocates memory for new path
        new_absolute_path = malloc(CHAR_BUF * sizeof(char));
        error_handler_null(new_absolute_path, NULL, "Memory for new path couldn't be allocated",
                           true);
        make_path(new_absolute_path, dir_struct->d_name, absolute_path);

        struct stat new_absolute_path_buf;
        int check = lstat(new_absolute_path, &new_absolute_path_buf);

        /**
         * if path is not readable, size of current directory is added.
         * As soon as the new absolute path wont be a part of a new task it gets free'd
         */
        if ((check < 0)) {
            block_size += (*absolute_path_buf).st_blocks;
            free(new_absolute_path);
            break;
        }
        else if (strcmp(dir_struct->d_name, ".") == 0) {
            block_size += new_absolute_path_buf.st_blocks;
            free(new_absolute_path);
        }
        else if (strcmp(dir_struct->d_name, "..") != 0) {
            //if path is a file
            if (S_ISREG(new_absolute_path_buf.st_mode)) {
                block_size += new_absolute_path_buf.st_blocks;
                free(new_absolute_path);
            }
            //if path is a directory
            else {
                //adds to task queue
                if(multithread) {
                    Task *new_task = create_task(new_absolute_path, (void (*)(struct task *,
                            Task_queue *)) (void (*)(void)) task->task_pointer);
                    add_task((Task_queue *)queue_or_permission, new_task);
                } else {
                    block_size += get_block_size(new_absolute_path, (bool *)queue_or_permission);
                    free(new_absolute_path);
                }
            }
        }
        else {
            free(new_absolute_path);
        }
    }
    error_handler_value(0, closedir(dir), "Couldn't close directory\n",
                        NULL, false);
    return block_size;
}


/**
 * @brief                                      Responsible for telling the threadpool to shutdown.
 *
 *                                             Doesn't do much apart from changing the task queues shutdown variable
 *                                             to true.
 *
 * @param task                                 A task where the pathname will be set to NULL.
 * @param queue                                A pointer to a task queue, containing the shutdown variable.
 * @return                                     Returns -1 indicating that this function is only for shutting down.
 */
blkcnt_t shutdown_threads(Task *task, Task_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->shutdown = true;
    task->path = NULL;
    pthread_mutex_unlock(&queue->mutex);
    return -1;
}


/**
 * @brief                                      Parses flags that has been arguments to the program. If -j
 *                                             flag is set, an integer indicating the thread amount is also used.
 *
 * @param argc                                 Amount of parameters to the program.
 * @param argv                                 Array of strings, containing the names of the arguments.
 * @param thread_amount                        Pointer to an integer for containing the amount of threads.
 */
void flag_options(int argc, char *argv[], int *thread_amount) {
    int option;
    while ((option = getopt(argc, argv, "j:")) != -1) {
        switch (option) {
            case 'j':
                *thread_amount = atoi(optarg);
                break;
            default:
                break;
        }
    }
}


/**
 * @brief                                      Responsible for adding a task to the task queue, and signaling the
 *                                             threadpool when this has occurred.
 *
 * @param t_queue                              Pointer to a task queue.
 * @param task                                 Pointer to the task that will be added.
 */
void add_task(Task_queue *t_queue, Task *task) {
    pthread_mutex_lock(&t_queue->mutex);
    enqueue(t_queue, task);
    int check_signal = pthread_cond_signal(&t_queue->cond);
    error_handler_value(0, check_signal, NULL, "Error! cond_signal failed\n",
                        false);
    pthread_mutex_unlock(&t_queue->mutex);
}


/**
 * @brief                                      Responsible for running the threads, the main function of the
 *                                             threadpool.
 *
 *                                             When no task is in the queue, the threads wait for a condition variable
 *                                             to be signalled when a new task has been added.
 *
 * @param t_queue                              The task queue that the threadpool gets it's tasks from.
 * @return                                     returns NULL.
 */
void *run_thread(Task_queue *t_queue) {

    pthread_mutex_lock(&t_queue->mutex);
    //loops until a kill task has been added to the queue
    while (!t_queue->shutdown) {

        //the threads wait here until a task has been added, and a signal is sent
        while (queue_is_empty(t_queue)) {
            int check_wait = pthread_cond_wait(&t_queue->cond, &t_queue->mutex);
            error_handler_value(0, check_wait, NULL, "Error! cond_wait failed\n",
                                false);
        }
        //dequeues and runs a task
        Task *task = dequeue(t_queue);
        t_queue->t_running++;
        pthread_mutex_unlock(&t_queue->mutex);
        run_task(t_queue, task);
        pthread_mutex_lock(&t_queue->mutex);
        kill_task(task);
    }
    pthread_mutex_unlock(&t_queue->mutex);
    return NULL;
}


/**
 * @brief                                      Responsible for starting a task.
 *
 *                                             Runs the function pointed to, which is contained in the task.
 *                                             Also adds the block size returned onto the common block_size variable.
 *
 * @param t_queue                              The task queue which also contains settings.
 * @param task                                 The task that will run.
 */
void run_task(Task_queue *t_queue, Task *task) {
    blkcnt_t temp_block_size;
    //make sure that the function pointed to is not inside of a mutex
    temp_block_size = task->task_pointer(task, t_queue);
    pthread_mutex_lock(&t_queue->mutex);

    //checks if the task that has been run is a kill-task or a regular
    if (temp_block_size > -1) {
        t_queue->block_size += temp_block_size;
    }
    t_queue->t_running--;

    bool queue_empty = queue_is_empty(t_queue);
    int t_running = t_queue->t_running;

    pthread_mutex_unlock(&t_queue->mutex);
    kill_task_initializer(t_queue, temp_block_size, queue_empty, t_running);

}


/**
 * @brief                                      Creates tasks for shutting down the threadpool.
 *
 * @param t_queue                              Pointer to a task queue.
 * @param temp_block_size                      Makes sure that killing tasks only can be made from a regular task.
 * @param queue_empty                          True if the queue is empty.
 * @param t_running                            Amount of threads in the threadpool currently doing work.
 */
void kill_task_initializer(Task_queue *t_queue, blkcnt_t temp_block_size, bool queue_empty, int t_running) {
    if ( queue_empty && (t_running == 0) && (temp_block_size > -1) ) {
        //creates the same amount of kill threads as thread amount
        for (int i = 0; i < t_queue->thread_amount; i++) {
            Task *new_task = create_task(NULL,
                                         (void (*)(struct task *, Task_queue *)) (void (*)(void)) shutdown_threads);
            add_task(t_queue, new_task);
        }
    }
}


/**
 * @brief                                      Starts the threadpool, adds the first task, and joins all of the threads
 *                                             when done.
 *
 * @param t_queue                              Pointer to a task queue.
 * @param start_path                           Name of the start path.
 */
void run_mult_thread(Task_queue *t_queue, char *start_path) {
    pthread_t threads[t_queue->thread_amount];

    //creates the threads
    for (int i = 0; i < t_queue->thread_amount; i++) {
        int pthread_create_check = pthread_create(&threads[i], NULL, (void *(*)(void *)) run_thread, t_queue);
        error_handler_value(0, pthread_create_check, "Error! Couldn't create thread: ",
                            (char *) threads[i],false);
    }

    //start task
    char *path = malloc(CHAR_BUF * sizeof(char));
    error_handler_null(path, NULL, "Couldn't allocate memory for start task\n",
                       true);
    strcpy(path, start_path);
    Task *start_task = create_task(path, (void (*)(struct task *, Task_queue *)) (void (*)(void)) get_block_size_mult);
    add_task(t_queue, start_task);

    //join threads
    for (int i = 0; i < t_queue->thread_amount; i++) {
        int pthread_join_check = pthread_join(threads[i], NULL);
        error_handler_value(0, pthread_join_check, "Could not join thread: ",
                            (char *) threads[i], false);
    }
}


/**
 * @brief                                      Parses path names that has been arguments to the program.
 *
 * @param argc                                 Amount of arguments to the program.
 * @param argv                                 Array of strings containing the name of the program arguments.
 * @return                                     Pointer to a dynamically allocated list containing path names.
 */
List *path_name_parser(int argc, char *const *argv) {
    List *path_name_list = list_create();
    while (optind < argc) {
        ListPos last_pos = list_end(path_name_list);
        char *path_name = malloc(CHAR_BUF * sizeof(char));
        error_handler_null(path_name, NULL, "Path name couldn't be allocated\n",
                           true);
        strcpy(path_name, argv[optind]);
        list_insert(last_pos, path_name);
        optind++;
    }
    return path_name_list;
}


/**
 * @brief                                      Adds a path onto another path name, and stores it in new_path.
 *
 * @param new_path                             An empty buffer where the two path names will be added to.
 * @param name                                 The path name that will be added to the absolute_path.
 * @param absolute_path                        The path that the name will be added onto.
 */
void make_path(char *new_path, const char *name, const char *absolute_path) {
    int i = 0;
    int j = 0;
    while (absolute_path[i] != '\0') {
        new_path[i] = absolute_path[i];
        i++;
    }
    if (absolute_path[i - 1] != '/') {
        new_path[i] = '/';
        i++;
    }
    while (name[j] != '\0') {
        new_path[i] = name[j];
        i++;
        j++;
    }
    new_path[i] = '\0';
}

/**
 * @}
 */
