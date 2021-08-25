//
//  AsyncJob.cpp
//  utils
//
//  Created by huang_jiafa on 2019/4/12.
//  Copyright © 2019 alibaba. All rights reserved.
//

#include "AsyncJob.h"

#define LOG_TAG "AsyncJob"
#include "timer.h"

static int AsyncJobLive = -1;

namespace Cicada {
    AsyncJob AsyncJob::sInstance{};

    AsyncJob::AsyncJob()
    {
        mThread = NEW_AF_THREAD(runJobs);
        AsyncJobLive = 1;
    }

    AsyncJob::~AsyncJob()
    {
        AsyncJobLive = 0;
#ifdef WIN32
        mThread->forceStop();
#endif
        delete mThread;

        while (!mJobItems.empty()) {
            std::function<void()> func = mJobItems.front();
            func();
            mJobItems.pop();
        }

        int64_t currentTime = af_getsteady_ms();
        while (!mDelayJobItems.empty()) {
            const DelayJobItem &jobItem = mDelayJobItems.front();
            if (currentTime >= jobItem.mRunTime) {
                if (jobItem.mFunc) {
                    jobItem.mFunc();
                }
                mDelayJobItems.pop_front();
            } else {
                break;
            }
        }
    }

    AsyncJob *AsyncJob::Instance(void)
    {
        if (AsyncJobLive == 0) return nullptr;
        return &sInstance;
    }

    void AsyncJob::addJob(std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mJobItems.push(func);
        mThread->start();
        mSleepCond.notify_all();
    }

    int64_t AsyncJob::addDelayJob(int64_t delayMs, std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        ++mJobId;
        DelayJobItem item(mJobId, af_getsteady_ms() + delayMs, func);
        auto iter = std::upper_bound(mDelayJobItems.begin(), mDelayJobItems.end(), item);
        bool needWake = false;
        if (iter == mDelayJobItems.begin()) {
            needWake = true;
        }
        mDelayJobItems.insert(iter, item);
        mThread->start();
        if (needWake) {
            mSleepCond.notify_all();
        }
        return mJobId;
    }

    bool AsyncJob::removeDelayJob(int64_t jobId)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (jobId <= 0 || mDelayJobItems.empty()) {
            return false;
        }
        auto iter = std::find_if(mDelayJobItems.begin(), mDelayJobItems.end(),
                                 [jobId](const DelayJobItem &item) { return jobId == item.mJobId; });
        bool succ = false;
        if (iter != mDelayJobItems.end()) {
            mDelayJobItems.erase(iter);
            succ = true;
        }
        return succ;
    }

    int AsyncJob::runJobs()
    {
        std::function<void()> func = nullptr;
        int64_t sleepTime = 0;
        {
            std::lock_guard<std::mutex> lock(mMutex);

            if (!mJobItems.empty()) {
                func = mJobItems.front();
                mJobItems.pop();
            } else {
                if (!mDelayJobItems.empty()) {
                    int64_t currentTime = af_getsteady_ms();
                    const DelayJobItem &jobItem = mDelayJobItems.front();
                    if (currentTime >= jobItem.mRunTime) {
                        func = jobItem.mFunc;
                        mDelayJobItems.pop_front();
                    } else {
                        sleepTime = jobItem.mRunTime - currentTime;
                    }
                }
            }
        }

        if (nullptr != func) {
            func();
        }
        if (sleepTime > 0) {
            std::unique_lock<std::mutex> lock(mMutex);
            mSleepCond.wait_for(lock, std::chrono::milliseconds(sleepTime));
        }

        {
            std::lock_guard<std::mutex> lock(mMutex);
            bool haveMore = false;
            if (!mJobItems.empty() || !mDelayJobItems.empty()) {
                haveMore = true;
            }
            return haveMore ? 0 : -1;
        }
    }
}
