#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <stddef.h>

/**
 * @file Template.h
 * @brief Public API for the <MODULENAME> module.
 *
 * Provides data structures and functions for <kort beskrivning av modulens syfte>.
 */

/**
 * @defgroup <MODULENAME> <MODULENAME>
 * @brief <Kort beskrivning av modulen>
 *
 * <Längre beskrivning, t.ex. vad modulen hanterar, beroenden, eventuella begränsningar.>
 *
 * @note Data may include <specific notes>.
 * @note Internally performs <network I/O, parsing, caching etc.>
 * @{
 */

/**
 * @brief Example struct representing <vad structen representerar>.
 *
 * @note This struct owns its memory.
 * @warning Null-terminated strings must not be freed externally.
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
 * @note Performs <blocking I/O, parsing, etc.>
 * @warning Output structure must be pre-allocated.
 * @pre output != NULL
 * @post output contains valid data.
 */
int ExampleFunction(ExampleStruct *output, int param);

/** @} */

#endif