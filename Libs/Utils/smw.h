#ifndef __smw_h
#define __smw_h

#include <stdint.h>

/**
 * @file smw.h
 * @brief Simple task scheduler/worker module using callbacks.
 * @defgroup smw Simple Worker Module
 * @{
 */

#ifndef smw_max_tasks
#define smw_max_tasks 10
#endif

/**
 * @struct smw_task
 * @brief Represents a scheduled task.
 *
 * A task consists of a user-provided context pointer and a callback function.
 *
 * @note `context` is owned by the caller and not managed by this module.
 * @note `callback` must remain valid for the lifetime of the task.
 */
typedef struct {
  void *context; /**< User-defined context (not owned by module) */
  void (*callback)(void *context, uint64_t monTime); /**< Task callback */
} smw_task;

/**
 * @struct smw
 * @brief Container for all scheduled tasks.
 *
 * @note Uses a fixed-size array of tasks.
 * @note Maximum number of tasks is defined by `smw_max_tasks`.
 */
typedef struct {
  smw_task tasks[smw_max_tasks]; /**< Array of task slots */
} smw;

/**
 * @brief Global scheduler instance.
 *
 * @note This module uses a single global instance (singleton-like design).
 * @warning Not thread-safe.
 */
extern smw g_smw;

/**
 * @brief Initializes the scheduler.
 *
 * Clears all task slots.
 *
 * @return 0 on success.
 *
 * @post All task slots are reset to NULL.
 */
int smw_init();

/**
 * @brief Creates a new task.
 *
 * @param _Context User-defined context pointer.
 * @param _Callback Function to be executed.
 *
 * @return Pointer to created task, or NULL if no free slot is available.
 *
 * @pre `_Callback` must not be NULL.
 *
 * @post Task is registered in scheduler.
 *
 * @warning No bounds expansion; limited to `smw_max_tasks`.
 * @note Caller retains ownership of `_Context`.
 */
smw_task *smw_create_task(void *_Context,
                          void(_Callback)(void *context, uint64_t monTime));

/**
 * @brief Destroys a task.
 *
 * @param _Task Pointer to task.
 *
 * @post Task slot is cleared and becomes available.
 *
 * @note Safe to call with NULL (no effect).
 */
void smw_destroy_task(smw_task *_Task);

/**
 * @brief Executes all active tasks.
 *
 * @param monTime Monotonic time value passed to callbacks.
 *
 * @pre Scheduler must be initialized.
 *
 * @warning Callbacks are executed sequentially.
 * @warning No protection against callback side effects.
 */
void smw_work(uint64_t monTime);

/**
 * @brief Clears all tasks from scheduler.
 *
 * @post All task slots are reset.
 *
 * @note Does not free user-provided context.
 */
void smw_dispose();

/**
 * @brief Returns number of active tasks.
 *
 * @return Number of tasks with active callbacks.
 */
int smw_getTaskCount();

/** @} */

#endif // SMW_H