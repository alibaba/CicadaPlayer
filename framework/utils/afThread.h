//
// Created by moqi on 2018/9/3.
//

#ifndef FRAMEWORK_AFTHREAD_H
#define FRAMEWORK_AFTHREAD_H

#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "CicadaType.h"
#include <atomic>

#define NEW_AF_THREAD(func) (new afThread([this]() -> int { return this->func(); }, LOG_TAG))

class CICADA_CPLUS_EXTERN afThread {
public:
    typedef int (*thread_func)(void *arg);

    typedef std::function<int()> thread_beginCallback;
    typedef std::function<void()> thread_endCallback;

    typedef enum AF_THREAD_STATUS {
        THREAD_STATUS_IDLE = 0,
        THREAD_STATUS_STOPPED,
        THREAD_STATUS_RUNNING,
        THREAD_STATUS_PAUSED,
    } AF_THREAD_STATUS;

public:
    explicit afThread(std::function<int()> func, const char *name = "");

    ~afThread();

    int start();

    void pause();

    void prePause();

    void stop();

    void forceStop();

    AF_THREAD_STATUS getStatus()
    {
        return mThreadStatus;
    }

    void setBeginCallback(const thread_beginCallback &callback);

    void setEndCallback(const thread_endCallback &callback);
    //void detach();

private:
    static void threadRun(void *arg);

    void onRun();

private:
//    thread_func mFunc = nullptr;
    std::function<int()> mFunc;
    std::string mName = "";

    std::atomic_bool mWaitPaused{false};
    std::atomic_bool mTryPaused{false};
    std::mutex mSleepMutex;
    std::condition_variable mSleepCondition;

    std::thread *mThreadPtr = nullptr;
    std::mutex mMutex;

    thread_beginCallback mThreadBeginCallback = nullptr;
    thread_endCallback mThreadEndCallback = nullptr;

protected:
    std::atomic<AF_THREAD_STATUS> mThreadStatus{THREAD_STATUS_IDLE};
};


#endif //FRAMEWORK_AFTHREAD_H
