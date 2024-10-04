/**
 * @brief This module has the most common operations for the datatype list.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 3.0
 *
 */

#include <stdlib.h>
#include <string.h>
#include "error_handler.h"
#include "list.h"

static struct node *make_node(void *value) {
    struct node *temp = malloc(sizeof(struct node));
    error_handler_null(temp, NULL, "Node couldn't allocate memory", true);
    temp->value = value;
    temp->prev = NULL;
    temp->next = NULL;
    return temp;
}

List *list_create(void) {
    List *list = malloc(sizeof(List));
    error_handler_null(list, NULL, "list couldn't allocate memory", true);
    list->head.value = NULL;
    list->head.prev = &list->head;
    list->head.next = &list->head;
    return list;
}

void list_destroy(List *lst) {
    if (!list_is_empty(lst)) {
        ListPos pos = list_first(lst);
        while (pos.node != &lst->head) {
            pos = list_remove(pos);
        }
    }
    free(lst);
}

bool list_is_empty(const List *lst) {
    return (lst->head.next == &lst->head);
}


ListPos list_first(List *lst) {
    ListPos pos = {
        .node = lst->head.next
    };
    return pos;
}

ListPos list_end(List *lst) {
    ListPos pos = {
        .node = &lst->head
    };
    return pos;
}

bool list_pos_equal(ListPos p1, ListPos p2) {
    return p1.node == p2.node;
}

ListPos list_next(ListPos pos) {
    pos.node = pos.node->next;
    return pos;
}

ListPos list_prev(ListPos pos) {
    pos.node = pos.node->prev;
    return pos;
}

ListPos list_insert(ListPos pos, void *value) {
    // Create a new node.
    struct node *node = make_node(value);

    // Find nodes before and after (may be the same node: the head of the list).
    struct node *before = pos.node->prev;
    struct node *after = pos.node;

    // Link to node after.
    node->next = after;
    after->prev = node;

    // Link to node before.
    node->prev = before;
    before->next = node;

    // Return the position of the new element.
    pos.node = node;

    return pos;
}

ListPos list_remove(ListPos pos) {
    ListPos next_pos = list_next(pos);
    ListPos prev_pos = list_prev(pos);

    prev_pos.node->next = next_pos.node;
    next_pos.node->prev = prev_pos.node;
    free(pos.node->value);
    free(pos.node);
    return next_pos;
}

void *list_inspect(ListPos pos) {
    return pos.node->value;
}
