#include "task.h"

volatile bool task_running = false;
volatile bool task_kill_signal = false;

void (*task_function)(void) = NULL;

void task_stop()
{
    if (!task_running)
    {
        return;
    }

    task_kill_signal = true;

    // TODO: better handling to avoid deadlock
    while (task_running)
    {
        sleep_ms(10);
    }

    task_function = NULL;
    task_kill_signal = false;
}

void task_execute()
{
    if (task_running || task_function == NULL)
    {
        return;
    }

    task_running = true;
    task_function();
    task_running = false;
    task_function = NULL;
}

void task_start(void (*task)(void))
{
    task_kill_signal = false;

    if (task_running)
    {
        return;
    }

    task_function = task;

    multicore_reset_core1();
    multicore_launch_core1(task_execute);

    // TODO: better handling to avoid deadlock
    while (!task_running)
    {
    }
}

bool task_is_running()
{
    return task_running;
}
