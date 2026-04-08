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
 * @brief Internal helper used by ExampleFunction.
 *
 * @param input Input value.
 * @return Computed result.
 */
static int InternalHelper(int input)
{
    // Implementation here
    return input * 2;
}

/**
 * @brief Implementation of ExampleFunction.
 *
 * See header for full contract documentation.
 */
int ExampleFunction(ExampleStruct *output, int param)
{
    if (!output) return -1;

    output->value = InternalHelper(param);
    strncpy(output->name, "example", sizeof(output->name)-1);
    output->name[sizeof(output->name)-1] = '\0';

    return 0;
}

/**
 * @brief Prints an ExampleStruct for debugging.
 *
 * @param value Pointer to struct to print.
 */
void ExampleStruct_Print(const ExampleStruct *value)
{
    if (!value)
    {
        return;
    }

    printf("ExampleStruct: name=%s, value=%d\n", value->name, value->value);
}

// Additional internal or public functions here
