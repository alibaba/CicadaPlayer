//
// Created by moqi on 2018/9/19.
//
#include "utils/af_clock.h"
#include "frame_work_log.h"
#include "timer.h"

int main()
{
    af_scalable_clock clock;
    clock.start();
    int speed = 1;

    for (int i = 0; i < 20; i++) {
        if (i / 5 + 1 > speed) {
            speed = i / 5 + 1;
            AF_LOGD("speed is %d\n", speed);
        }
        clock.setSpeed(speed);
        AF_LOGD("cur clock is %f\n", (float) clock.get() / 1000000);
        af_msleep(100);
    }

    clock.set(30000000);
    AF_LOGD("cur clock is %f\n", (float) clock.get() / 1000000);

    for (int i = 20; i > 0; i--) {
        if (i / 5 + 1 != speed) {
            speed = i / 5 + 1;
            AF_LOGD("speed is %d\n", speed);
        }
        clock.setSpeed(speed);
        AF_LOGD("cur clock is %f\n", (float) clock.get() / 1000000);
        af_msleep(100);
    }
    for (int i = 20; i > 0; i--) {
        clock.pause();
        AF_LOGD("cur clock is %f\n", (float) clock.get() / 1000000);
        af_msleep(100);
    }

    clock.start();

    for (int i = 20; i > 0; i--) {
        if (i / 5 + 1 != speed) {
            speed = i / 5 + 1;
            AF_LOGD("speed is %d\n", speed);
        }
        clock.setSpeed(-speed);
        AF_LOGD("cur clock is %f\n", (float) clock.get() / 1000000);
        af_msleep(100);
    }
}