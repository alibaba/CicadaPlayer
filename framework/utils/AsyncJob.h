//
//  AsyncJob.h
//  utils
//
//  Created by huang_jiafa on 2019/4/12.
//  Copyright © 2019 alibaba. All rights reserved.
//

#ifndef AsyncJob_h
#define AsyncJob_h

#include "afThread.h"
#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>

namespace Cicada {

    class DelayJobItem {
    public:
        DelayJobItem(int64_t jobId, int64_t runTime, std::function<void()> func) : mJobId(jobId), mRunTime(runTime), mFunc(func)
        {}

        bool operator<(const DelayJobItem &item) const
        {
            return mRunTime < item.mRunTime;
        }

    public:
        int64_t mJobId{0};
        int64_t mRunTime{0};
        std::function<void()> mFunc{nullptr};
    };

    class AsyncJob {
    public:
        static AsyncJob *Instance(void);
        void addJob(std::function<void()> func);

        int64_t addDelayJob(int64_t delayMs, std::function<void()> func);
        bool removeDelayJob(int64_t jobId);

    private:
        AsyncJob();

        ~AsyncJob();

        int runJobs();

    private:
        static AsyncJob sInstance;
        afThread *mThread = nullptr;
        std::mutex mMutex;
        std::queue<std::function<void()>> mJobItems;
        std::list<DelayJobItem> mDelayJobItems;
        int64_t mJobId{0};
        std::condition_variable mSleepCond{};
    };

}// namespace Cicada

#endif /* AliAsyncJob_h */
