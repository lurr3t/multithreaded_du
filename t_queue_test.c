//
// Created by Ludwig Fallström on 2021-11-08.
//
#include "list.h"
#include "t_queue.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH 4096

void eksde() {
    printf("hej hopp\n");
}

int main(void) {

    //test creation
    Task_queue *queue = create_task_queue(10);

    for (int i = 0; i < 10; i++) {
        char *temp_path = malloc(MAX_PATH * sizeof(char));
        strcpy(temp_path, "eksde");
        Task *task = create_task(temp_path, (void *)eksde);
        enqueue(queue, task);
    }
    Task *outside_task = dequeue(queue);
    printf("%s\n", outside_task->path);

    char *temp_path = malloc(MAX_PATH * sizeof(char));
    strcpy(temp_path, "eksde");
    Task *task = create_task(temp_path, (void *)eksde);
    enqueue(queue, task);

    kill_task(outside_task);
    destroy_queue(queue);
    return 0;
}
