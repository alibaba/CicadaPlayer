//
// Created by moqi on 2018/12/4.
//

#include <cstring>
#include "afThread.h"
#include "frame_work_log.h"
#include "timer.h"

typedef struct thread_test_t {
    char *p;
    int ret;

} thread_test;

static int test_func(void *arg)
{
    thread_test *data = (thread_test *) arg;
    af_msleep(10);
    AF_LOGD("%s", data->p);
  //  return --data->ret;
    return 0;

}
int main()
{
    thread_test *data = (thread_test *) malloc(sizeof(thread_test));
    memset(data, 0, sizeof(thread_test));
    data->p = static_cast<char *>(malloc(16));
    strcpy(data->p, "afTreadTest");
    auto func = [data] ()->int { return test_func(data);};
    afThread *t = new afThread(func, "afTreadTest");
    t->start();
    AF_TRACE;
    for (int i = 0; i <= 100; i++) {
        if (i % 2) {
            t->start();
        } else {
            t->pause();
        }
    }
    af_msleep(100);
//    data->ret = 10000;
//    af_msleep(100);
//    AF_TRACE;
//    t->start();
//    af_msleep(100);
//    t->pause();
//    free(data->p);
//    data->p = nullptr;
//    data->p = strdup("afTreadTest1");
//    t->start();
//    //   t->detach();
//    af_msleep(100);
//    AF_TRACE;
    delete t;
//    free(data->p);
//    free(data);
//    AF_TRACE;
//    af_msleep(100);
}

