//
// Created by moqi on 2018/5/12.
//
#include <ctime>
#include <chrono>
#include <thread>
#include "timer.h"

int64_t af_gettime_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t af_getsteady_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

int64_t af_gettime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t af_gettime_relative()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void af_msleep(int ms)
{
    std::chrono::milliseconds dura(ms);
    std::this_thread::sleep_for(dura);
}

void af_usleep(int us)
{
    std::chrono::microseconds dura(us);
    std::this_thread::sleep_for(dura);
}

int af_make_abstime_latems(struct timespec *pAbstime, uint32_t ms)
{
    int64_t t = af_gettime();
    pAbstime->tv_sec = t / 1000000 + (ms / 1000);
    pAbstime->tv_nsec = ((t % 1000000) + (ms % 1000) * 1000) * 1000;
    if (pAbstime->tv_nsec > 1000000000l) {
        pAbstime->tv_sec += 1;
        pAbstime->tv_nsec -= 1000000000l;
    }
    return 0;
}
