#ifndef _TASK_H_
#define _TASK_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

typedef enum task_type_t
{
    task_type_float,
    task_type_int16,
    task_type_str,
    task_type_bool,
} task_type_t;

typedef union
{
    float f;
    uint16_t i;
    char * s;
    bool b;
} task_data_t;

typedef struct task_t
{
    void (*task_fn)(void);                      /* Function to execute */
    double period;                              /* How often to execute */
    time_t clock;                               /* keep track of time between executions */
} task_t;

void Task_Init(task_t * task_list, uint8_t num);
void Task_RunSingle(void);
void Task_RunAll(void);

#endif /* _TASK_H_ */

