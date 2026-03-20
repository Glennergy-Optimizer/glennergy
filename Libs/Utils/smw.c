/**
 * @file smw.c
 * @brief Implementation of simple task scheduler/worker.
 * @ingroup smw
 */

#include "smw.h"
#include <string.h>

/**
 * @brief Global scheduler instance.
 */
smw g_smw;

/**
 * @brief Initializes the scheduler.
 */
int smw_init()
{
    memset(&g_smw, 0, sizeof(smw));

    int i;
    for(i = 0; i < smw_max_tasks; i++)
    {
        g_smw.tasks[i].context = NULL;
        g_smw.tasks[i].callback = NULL;
    }

    return 0;

    // Suggestion: memset already clears struct; loop is redundant
}

/**
 * @brief Creates a task.
 */
smw_task* smw_create_task(void* _Context, void (*_Callback)(void* context, uint64_t monTime))
{
    int i;
    for(i = 0; i < smw_max_tasks; i++)
    {
        if(g_smw.tasks[i].context == NULL && g_smw.tasks[i].callback == NULL)
        {
            g_smw.tasks[i].context = _Context;
            g_smw.tasks[i].callback = _Callback;
            return &g_smw.tasks[i];
        }
    }

    return NULL;

    // Suggestion: Validate _Callback != NULL before assigning
}

/**
 * @brief Destroys a task.
 */
void smw_destroy_task(smw_task* _Task)
{
    if(_Task == NULL)
    {
        return;
    }

    int i;
    for(i = 0; i < smw_max_tasks; i++)
    {
        if(&g_smw.tasks[i] == _Task)
        {
            g_smw.tasks[i].context = NULL;
            g_smw.tasks[i].callback = NULL;
            break;
        }
    }

    // Suggestion: Return status if task was not found
}

/**
 * @brief Executes all tasks.
 */
void smw_work(uint64_t monTime)
{
    int i;
    for(i = 0; i < smw_max_tasks; i++)
    {
        if(g_smw.tasks[i].callback != NULL)
            g_smw.tasks[i].callback(g_smw.tasks[i].context, monTime);
    }

    // Suggestion: Consider handling long-running callbacks (blocking risk)
}

/**
 * @brief Returns number of active tasks.
 */
int smw_getTaskCount()
{
	int counter = 0;
	int i;
	for(i = 0; i < smw_max_tasks; i++)
	{
		if(g_smw.tasks[i].callback != NULL)
			counter++;
	}

	return counter;
}

/**
 * @brief Clears all tasks.
 */
void smw_dispose()
{
    int i;
    for(i = 0; i < smw_max_tasks; i++)
    {
        g_smw.tasks[i].context = NULL;
        g_smw.tasks[i].callback = NULL;
    }

    // Suggestion: Could reuse smw_init() for consistency
}