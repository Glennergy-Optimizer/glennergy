/**
 * @file TemplateMain.c
 * @brief Entry point / test for <MODULENAME> module.
 *
 * @ingroup <MODULENAME>
 *
 * Demonstrates usage of <MODULENAME> functions and structures.
 *
 * @note Only used for testing or demo.
 */

#define MODULE_NAME "MAIN"
#include "<MODULENAME>.h"
#include "../../Server/Log/Logger.h"
#include <stdio.h>
#include <stdlib.h>

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