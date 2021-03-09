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
    UTCTimer utcTime("2021-03-09T06:35:09Z");

    utcTime.start();
    int index = 0;
    while (++index < 10) {
        string time = (string) utcTime;
        cout << time << endl;
        af_msleep(1000);
    }
}