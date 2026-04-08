#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

/**
 * @file Template.h
 * @brief Public API for the <MODULENAME> module.
 *
 * Provides the public types and functions for <short module purpose>.
 */

/**
 * @defgroup <MODULENAME> <MODULENAME>
 * @brief <Short module description>
 *
 * <Longer description of what the module manages and important limitations
 * if needed.>
 *
 * @note Add notes only when they communicate meaningful ownership, blocking,
 * thread, or lifecycle behavior.
 * @{
 */

/**
 * @brief Example struct representing <what the struct models>.
 *
 * Describe key semantics at the struct level and keep individual field
 * comments short unless a field is subtle or safety-critical.
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

/**
 * @brief Prints an ExampleStruct for debugging.
 *
 * @param value Pointer to struct to print.
 */
void ExampleStruct_Print(const ExampleStruct *value);

/** @} */

#endif
