#include <pthread.h>
#include <utility>

//
// Created by moqi on 2018/9/3.
//

#define LOG_TAG "afThread"
#include "afThread.h"
#include "frame_work_log.h"
#include "timer.h"
#include <cassert>

#ifdef ANDROID

    #include <sys/prctl.h>
    #include <cassert>


    #define MAX_TASK_NAME_LEN (16)
#endif

#ifdef ANDROID

static void get_name(char *name)
{
    if (prctl(PR_GET_NAME, (unsigned long) name, 0, 0, 0) != 0) {
        strcpy(name, "<name unknown>");
    } else {
        // short names are null terminated by prctl, but the man page
        // implies that 16 byte names are not.
        name[MAX_TASK_NAME_LEN] = 0;
    }

    AF_LOGD("thread_name is %s\n", name);
}

static void set_name(char *name)
{
    char threadName[MAX_TASK_NAME_LEN + 1];
    size_t size = strlen(name) > MAX_TASK_NAME_LEN ? MAX_TASK_NAME_LEN : strlen(name);
    strncpy(threadName, name, size);
    threadName[size] = 0;
    prctl(PR_SET_NAME, (unsigned long) threadName, 0, 0, 0);
}

#endif

static void thread_set_self_name(char *name)
{
#if defined __APPLE__
    pthread_setname_np(name);
#elif defined(ANDROID)
    set_name(name);
#endif
}
afThread::afThread(std::function<int()> func, const char *name)
    : mFunc(std::move(func)),
      mName(name)
{
}

int afThread::start()
{
    std::unique_lock<std::mutex> uMutex(mMutex);
    mTryPaused = false;

    if (nullptr == mThreadPtr) {
        mThreadStatus = THREAD_STATUS_RUNNING;
        mThreadPtr = new std::thread(threadRun, this);
    } else {
        std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
        mThreadStatus = THREAD_STATUS_RUNNING;
        mSleepCondition.notify_one();
    }

    return 0;
}

void afThread::threadRun(void *arg)
{
    auto *pThread = static_cast<afThread *>(arg);
    pThread->onRun();
}

void afThread::onRun()
{
    if (mThreadBeginCallback != nullptr) {
        mThreadBeginCallback();
    }

    assert(mFunc != nullptr);
    int ret;

    if (mName.length() > 0) {
        thread_set_self_name(const_cast<char *>(mName.c_str()));
    }

    while (THREAD_STATUS_STOPPED < mThreadStatus) {
        if (mWaitPaused) {
            mThreadStatus = THREAD_STATUS_PAUSED;
            std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
            mWaitPaused = false;
            mSleepCondition.notify_one();
        }

        if (THREAD_STATUS_PAUSED == mThreadStatus) {
            std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
            // make sure notify on time, reduce loop cpu usage.
            mSleepCondition.wait(sleepMutex, [this]() {
                return THREAD_STATUS_PAUSED != mThreadStatus;
            });
        } else if (THREAD_STATUS_RUNNING == mThreadStatus) {
            ret = mFunc();

            if (ret < 0) {
                mTryPaused = true;
            }
        }

        if (mTryPaused) {
            if (mMutex.try_lock()) {
                mThreadStatus = THREAD_STATUS_PAUSED;
                mMutex.unlock();
            }

            mTryPaused = false;
        }
    }

    if (mThreadEndCallback != nullptr) {
        mThreadEndCallback();
    }
}

void afThread::prePause()
{
    if (mMutex.try_lock()) {
        if (THREAD_STATUS_RUNNING == mThreadStatus) {
            mTryPaused = true;
        }

        mMutex.unlock();
    }
}

void afThread::pause()
{
    std::unique_lock<std::mutex> uMutex(mMutex);

    if (THREAD_STATUS_RUNNING == mThreadStatus) {
        std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
        mWaitPaused = true;
        mSleepCondition.wait(sleepMutex, [this]() {
            return !mWaitPaused;
        });
    }
}

void afThread::stop()
{
    AF_TRACE;
    std::unique_lock<std::mutex> uMutex(mMutex);
    mTryPaused = false;
    {
        std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
        mThreadStatus = THREAD_STATUS_STOPPED;
    }
    mSleepCondition.notify_one();

    if (mThreadPtr && mThreadPtr->joinable()) {
        mThreadPtr->join();
    }

    delete mThreadPtr;
    mThreadPtr = nullptr;
    AF_TRACE;
}

//void afThread::detach()
//{
//    std::unique_lock<std::mutex> uMutex(mMutex);
//    if (mThreadPtr->joinable())
//        mThreadPtr->detach();
//}

afThread::~afThread()
{
    std::unique_lock<std::mutex> uMutex(mMutex);
    mTryPaused = false;
    {
        std::unique_lock<std::mutex> sleepMutex(mSleepMutex);
        mThreadStatus = THREAD_STATUS_IDLE;
    }
    mSleepCondition.notify_one();

    if (mThreadPtr && mThreadPtr->joinable()) {
        mThreadPtr->join();
    }

    delete mThreadPtr;
}

void afThread::setBeginCallback(const thread_beginCallback &callback)
{
    mThreadBeginCallback = callback;
}

void afThread::setEndCallback(const thread_endCallback &callback)
{
    mThreadEndCallback = callback;
}
