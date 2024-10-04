/**
 * @defgroup error_handler_h error_handler
 *
 * @brief This datatype has operations for handling errors, either when the error indication is NULL
 * or less than a certain value.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 1.0
 *
 * @{
 */

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

/**
 * @brief                               Handles errors when the error indication is NULL.
 *
 * @param structure_to_be_checked       Pointer to a structure that the error handling will be done upon.
 * @param exit_message                  A message with the error. NULL if no message is wanted.
 * @param error_name                    A name associated to the error. NULL if no name is wanted.
 * @param changed_errno                 True if error should be printed from errno.
 */
void error_handler_null(void *structure_to_be_checked, char *exit_message,
                        char *error_name, bool changed_errno);

/**
 * @brief                               Handles errors when the value is LESS than the check_against_value
 *
 * @param check_against_value           A value that the value will be checked against.
 * @param value                         The value that the error checking will be done upon.
 * @param exit_message                  A message with the error. NULL if no message is wanted.
 * @param error_name                    A name associated to the error. NULL if no name is wanted.
 * @param changed_errno                 True if error should be printed from errno.
 */
void error_handler_value(int check_against_value, int value, char *exit_message,
                         char *error_name, bool changed_errno);

#endif //ERROR_HANDLER_H

/**
 * @}
 */
