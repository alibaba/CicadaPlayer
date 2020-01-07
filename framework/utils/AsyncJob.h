//
//  AsyncJob.h
//  utils
//
//  Created by huang_jiafa on 2019/4/12.
//  Copyright © 2019 alibaba. All rights reserved.
//

#ifndef AsyncJob_h
#define AsyncJob_h

#include <mutex>
#include <queue>
#include <condition_variable>
#include "afThread.h"

namespace Cicada{

    class AsyncJob {
    public:
        static AsyncJob *Instance(void);
        void addJob(std::function<void()> func);
        
    private:
        AsyncJob();

        ~AsyncJob();

        int runJobs();

    private:
        static AsyncJob sInstance;
        afThread* mThread = nullptr;
        std::mutex mMutex;
        std::queue< std::function<void()> > mFuncs;
    };

}// namespace Cicada

#endif /* AliAsyncJob_h */
