//
// Created by moqi on 2018/11/27.
//
#include <pthread.h>
#include "property.h"
#include "frame_work_log.h"

static void propertyTest(const char *key, const char *value)
{
    setProperty(key, value);
    AF_LOGD("%s value is %s\n", key, getProperty(key));
}

static void *thread_test(void *arg)
{
    propertyTest("protected.test", "1234");
    propertyTest("protected.test", "1111");
    return NULL;
}

int main()
{
    propertyTest("ro.test", "1234");
    propertyTest("ro.test", "1111");

    propertyTest("protected.test", "1234");
    propertyTest("protected.test", "1111");

    pthread_t id;
    pthread_create(&id, NULL, thread_test, NULL);
    pthread_join(id, NULL);

    propertyTest("test.test", "1234");
    propertyTest("test.test", "1111");


}

