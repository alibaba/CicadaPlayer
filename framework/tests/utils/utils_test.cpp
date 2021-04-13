//
// Created by pingkai on 2021/3/9.
//

#include "gtest/gtest.h"
#include <string>
#include <utils/UTCTimer.h>
#include <utils/timer.h>
using namespace Cicada;
using namespace std;
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(time, utcTime)
{
    UTCTimer utcTime("2021-04-13T01:34:44.130Z");

    utcTime.start();
    int index = 0;
    while (++index < 10) {
        string time = (string) utcTime;
        cout << time << endl;
        af_msleep(1000);
    }
}
TEST(time, NTPTime)
{
    NTPClient ntpClient{};
    int64_t time;

    do {
        time = ntpClient.get();
        if (time == -EAGAIN) {
            usleep(10000);
        }
    } while (time == -EAGAIN);

    UTCTimer utcTime((string) ntpClient);
    utcTime.start();
    int index = 0;
    while (++index < 10) {
        cout << (string) utcTime << endl;
        af_msleep(1000);
    }
}