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

        while (0 < mFuncs.size()) {
            std::function<void()> func = mFuncs.front();
            func();
            mFuncs.pop();
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
        mFuncs.push(func);
        mThread->start();
    }

    int AsyncJob::runJobs()
    {
        bool haveMore = false;
        std::function<void()> func = nullptr;
        {
            std::lock_guard<std::mutex> lock(mMutex);

            if (mFuncs.size() > 0) {
                func = mFuncs.front();
                mFuncs.pop();
            }

            if (mFuncs.size() > 0) {
                haveMore = true;
            }
        }

        if (nullptr != func) {
            func();
        }

        return haveMore ? 0 : -1;
    }
}
