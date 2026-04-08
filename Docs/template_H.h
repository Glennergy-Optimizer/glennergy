#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

/**
 * @file Template.h
 * @brief Public API for the TEMPLATE module.
 */

/**
 * @defgroup TEMPLATE TEMPLATE
 * @brief Example module API.
 *
 * Demonstrates the repository documentation style for a small public header.
 * @{
 */

/**
 * @brief Example struct representing a simple data pair.
 *
 * Keeps a name and an integer value together.
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
