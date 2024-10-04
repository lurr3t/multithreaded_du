/**
 * @defgroup list_h list
 *
 * @brief This module has the most common operations for the datatype list.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 3.0
 *
 * @{
 */

#ifndef LIST_H
#define LIST_H

 /**
  * @brief                    The datatype for a struct node.
  *
  *                           A node in a linked list.
  *
  * @elem next                A pointer to the next node in the list.
  * @elem prev                A pointer to the previous node in the list.
  * @elem value               A void pointer for storing a value of a user defined type.
  *
  */
struct node {
    struct node *next;
    struct node *prev;
    void *value;
};

/**
 * @brief                     The datatype for a list.
 *
 *                            Contains the head of the list (start node of the list)
 *
 * @elem head                 The first node in the list. Contains no value.
 *
 */
typedef struct list {
    struct node head;
} List;

/*
 * @brief                     The datatype for a list position.
 *
 *                            The position for a node in the list.
 *
 * @elem node                 A pointer to a struct node.
 *
 */
typedef struct list_pos {
    struct node *node;
} ListPos;

/**
 * @brief                     Creates a new list.
 *
 *                            Creates the head of the linked list, and initializes it's values with null or zero.
 *
 * @return                    Returns a pointer to a allocated list.
 *
 */
List *list_create(void);

/**
 * @brief                     Destroys the list and deallocates all of it's values.
 *
 * @param lst                 A pointer to a list that will be destroyed.
 *
 * @return                    Destroys the list that lst is pointing to.
 *
 */
void list_destroy(List *lst);

/**
 * @brief                     Checks if the list is empty of elements.
 *
 * @param lst                 A pointer to a list.
 *
 * @return                    True of the list is empty.
 *
 */
bool list_is_empty(const List *lst);

/**
 * @brief                     Gives the list's first position.
 *
 * @param lst                 A pointer to a list.
 *
 * @return                    The first position of the list, of the datatype ListPos
 *
 */
ListPos list_first(List *lst);

/**
 * @brief                     Gives the list position after the last node.
 *
 * @param lst                 A pointer to the datatype list.
 *
 * @return                    The list's position after the last element, of the type ListPos.
 *
 */
ListPos list_end(List *lst);

/**
 * @brief                     Checks if the position is pointing towards the same node.
 *
 * @param p1                  The position of a node.
 * @param p2                  The position of a node.
 *
 * @return                    True if the positions is pointing towards the same node.
 *
 */
bool list_pos_equal(ListPos p1, ListPos p2);

/**
 * @brief                     Goes one position forwards in the list.
 *
 *                            Is not defined for the last position in the list.
 *
 * @param pos                 Variable of the type ListPos.
 *
 * @return                    The next position in the list.
 *
 */
ListPos list_next(ListPos pos);

/**
 * @brief                     Goes one position backwards in the list.
 *
 *                            Is not defined for the first position in the list.
 *
 * @param pos                 Variable of the type ListPos.
 *
 * @return                    The previous position in the list.
 *
 */
ListPos list_prev(ListPos pos);

/**
 * @brief                     Inserts a value in the list.
 *
 *                            Inserts the value right before the pos position.
 *
 * @param pos                 Variable of the type ListPos.
 * @param value               A void pointer to a user defined value type.
 *
 * @return                    The position of the new node.
 *
 */
ListPos list_insert(ListPos pos, void *value);

/**
 * @brief                     Removes a value from the list.
 *
 *                            Removes the value and the node in the position pos. Is not defined
 *                            for the list's last position. Also deallocates the memory for the value and the node.
 *                            The user is responsible for deallocating eventual memory inside of the value.
 *
 * @param pos                 The position where the node will be removed.
 *
 * @return                    The position of the node that comes after the removed node.
 *
 */
ListPos list_remove(ListPos pos);

/**
 * @brief                     Returns the value in the node at the pos position.
 *
 * @param pos                 Position of a node that the value will bre retrieved from.
 *
 * @return                    A void pointer pointing to the value structure.
 *
 */
void *list_inspect(ListPos pos);

#endif /* LIST_H */

/**
 * @}
 */
