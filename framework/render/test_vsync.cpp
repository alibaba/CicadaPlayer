//
// Created by moqi on 2019-08-05.
//


#include <utils/timer.h>
#include <utils/frame_work_log.h>
#include <render/video/vsync/CADisplayLinkVSync.h>
#include "render/video/vsync/timedVSync.h"


class testListener : public IVSync::Listener
{
public:

    int VSyncOnInit() override
    {
        return 0;
    }


    void VSyncOnDestroy() override
    {
    }
    int onVSync(int64_t tick) override
    {
        if (last != INT32_MIN) {
            count++;
        }

        last = af_getsteady_ms();
        int ms = 10 + rand() % 5;
        int64_t start = af_gettime_relative();
        int64_t end = start;

        do {
            af_usleep(1000);
            end = af_gettime_relative();
        } while (end - start < (ms) * 1000);

        AF_LOGI("sleep for %d\n", af_getsteady_ms() - last);
    }

public:
    int last = INT32_MIN;
    int count = 0;
};

int main()
{
    testListener listener{};
    CADisplayLinkVSync vSync(listener);
    auto start = af_gettime_relative();
    vSync.start();
    af_msleep(1000);
    vSync.pause();
    auto end = af_gettime_relative();
    AF_LOGI("avg is %d\n", (end - start) / listener.count);
}

