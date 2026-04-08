/**
 * @file TemplateMain.c
 * @brief Test entry point for the <MODULENAME> module.
 *
 * @ingroup <MODULENAME>
 *
 * Demonstrates fetching or preparing module data and passing it through the
 * integration path used by the consumer process.
 *
 * @note Intended for testing and integration rather than core business logic.
 * @warning Mention blocking I/O or similar runtime caveats only when they are
 * truly relevant to the file.
 */

#define MODULE_NAME "MAIN"
#include "<MODULENAME>.h"
#include "../../Server/Log/Logger.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Runs the <MODULENAME> test flow.
 *
 * Initializes dependencies, executes the module test path, and reports the
 * result.
 *
 * @return
 * - 0 on success
 * - Negative error code on failure
 */
int main(void)
{
    log_Init("example.log");

    ExampleStruct example;
    int rc = ExampleFunction(&example, 42);
    if (rc != 0)
    {
        LOG_ERROR("Failed to run ExampleFunction\n");
        return -1;
    }

    printf("ExampleStruct: name=%s, value=%d\n", example.name, example.value);

    log_Cleanup();
    return 0;
}
