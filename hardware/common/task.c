
#include "task.h"

static task_t * task;
static uint8_t num_tasks = 0;

static time_t currentTime;

void Task_Init(task_t * task_list, uint8_t num)
{
    task = task_list;
    num_tasks = num;
    time(&currentTime);
}
void Task_RunSingle(void)
{
    static uint8_t idx;
    time(&currentTime);
    double delta = difftime( currentTime, task[idx].clock);
    if( delta > task[idx].period)
    {
        task[idx].task_fn();
        task[idx].clock = currentTime;
    }
    idx++;
    idx%=num_tasks;
}
void Task_RunAll(void)
{

}

