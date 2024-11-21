#pragma once
#include <stdio.h>
#include "pico/stdlib.h"
#include <pico/multicore.h>

bool task_is_running();
void task_execute();
void task_stop();
void task_start(void (*task)(void));
