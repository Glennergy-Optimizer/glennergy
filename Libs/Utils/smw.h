#ifndef __smw_h
#define __smw_h

#include <stdint.h>

#ifndef smw_max_tasks
#define smw_max_tasks 10
#endif

typedef struct {
  void *context;
  void (*callback)(void *context, uint64_t monTime);
} smw_task;

typedef struct {
  smw_task tasks[smw_max_tasks];
} smw;

extern smw g_smw;

int smw_init();

smw_task *smw_create_task(void *_Context,
                          void(_Callback)(void *context, uint64_t monTime));
void smw_destroy_task(smw_task *_Task);

void smw_work(uint64_t monTime);

void smw_dispose();

int smw_getTaskCount();
#endif // SMH_H
