/**
 * @file Template.c
 * @brief Implementation of the <MODULENAME> module.
 *
 * @ingroup <MODULENAME>
 */

#define MODULE_NAME "<MODULENAME>"
#include "<MODULENAME>.h"
#include "../../Server/Log/Logger.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Internal helper function (example).
 *
 * Performs <kort beskrivning av funktionen>.
 *
 * @param[in] input Some input parameter
 * @return Computed result
 *
 * @note Only for internal use.
 * @warning Do not call externally.
 */
static int InternalHelper(int input)
{
    // Implementation here
    return input * 2;
}

/**
 * @brief ExampleFunction implementation.
 *
 * See header for full documentation.
 */
int ExampleFunction(ExampleStruct *output, int param)
{
    if (!output) return -1;

    output->value = InternalHelper(param);
    strncpy(output->name, "example", sizeof(output->name)-1);
    output->name[sizeof(output->name)-1] = '\0';

    return 0;
}

// Additional internal or public functions here