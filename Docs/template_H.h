#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

/**
 * @file Template.h
 * @brief Public API template for a module.
 *
 * This file demonstrates the repository's header documentation standard for
 * public types and functions.
 */

/**
 * @defgroup TEMPLATE TEMPLATE
 * @brief Template documentation group.
 *
 * Example group for documenting a module's public API, including its types,
 * functions, and any relevant ownership or lifecycle constraints.
 *
 * @note Add notes only when they communicate meaningful ownership, blocking,
 * thread, or lifecycle behavior.
 * @{
 */

/**
 * @brief Example struct representing a simple data record.
 *
 * @note Describe ownership or lifetime only when it matters.
 */
typedef struct {
    char name[32]; /**< Example string field. */
    int value;     /**< Example numeric field. */
} ExampleStruct;

/**
 * @brief Example function.
 *
 * @param[out] output Pointer to pre-allocated struct to populate.
 * @param[in] param Example input parameter.
 *
 * @return
 * - 0 on success
 * - -1 on error
 *
 * @note Add ownership, blocking, or lifecycle notes only when useful.
 * @warning Add warnings only when misuse or side effects are meaningful.
 * @pre Add only if the requirement is not obvious from the signature.
 * @post Add only if there is a meaningful postcondition to communicate.
 */
int ExampleFunction(ExampleStruct *output, int param);

/** @} */

#endif
