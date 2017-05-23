#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "timer.h"

void Timer::start()
{
    gettimeofday(&tbegin, &zone);
}

void Timer::stop()
{
    gettimeofday(&tend, &zone);
}

float Timer::peek()
{
    float total_time;
    long time_in_sec, time_in_ms;

    time_in_sec = tend.tv_sec - tbegin.tv_sec;
    time_in_ms = tend.tv_usec - tbegin.tv_usec;
    total_time = ((float) time_in_ms)/1000000.0;
    total_time += ((float) time_in_sec);
    return total_time;
}
