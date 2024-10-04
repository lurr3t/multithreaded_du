/**
 * @brief This datatype has operations for handling errors, either when the error indication is NULL
 * or less than a certain value.
 *
 * @author  Ludwig Fallström
 * @since   2021-11-16
 * @version 1.0
 *
 */

#include "error_handler.h"

void error_message_handler(const char *exit_message, const char *error_name, bool changed_errno);


void error_handler_null(void *structure_to_be_checked, char *exit_message,
                        char *error_name, bool changed_errno) {
    if (structure_to_be_checked == NULL) {
        error_message_handler(exit_message, error_name, changed_errno);
    }
}

void error_handler_value(int check_against_value, int value, char *exit_message,
                         char *error_name, bool changed_errno) {
    if (value < check_against_value) {
        error_message_handler(exit_message, error_name, changed_errno);
    }
}

/**
 * @brief                               Handles the printing of error messages. Also exit's the program
 *                                      with exit code EXIT_FAILURE.
 *
 * @param exit_message                  A message with the error. NULL if no message is wanted.
 * @param error_name                    A name associated to the error. NULL if no name is wanted.
 * @param changed_errno                 True if error should be printed from errno.
 */
void error_message_handler(const char *exit_message, const char *error_name, bool changed_errno) {

    if (changed_errno) {
        if (error_name == NULL) { perror(exit_message); }
        else { perror(error_name); }
    } else if ((error_name != NULL) && (exit_message != NULL)) {
        fprintf(stderr, exit_message, error_name);
    } else if (error_name == NULL) {
        fprintf(stderr, "%s", exit_message);
    } else {
        fprintf(stderr, "%s", error_name);
    }
    exit(EXIT_FAILURE);
}

