//
// Created by moqi on 2019-01-02.
//

#ifndef FRAMEWORK_ISLICEMANAGER_H
#define FRAMEWORK_ISLICEMANAGER_H

#include "memPool.h"
#include "memPoolSlice.h"
#include <mutex>
using namespace std;

namespace Cicada{
    class ISliceManager {

    public:
        static ISliceManager *getManager();

        int getSliceSize();

        ~ISliceManager();

        slice *getSlice(uint64_t capacity, uint64_t position, SliceReleaseCb &release);

        void returnSlice(slice *pSlice);
        
        void updateSliceUseTime(slice* pSlice);

    private:
        ISliceManager();

    private:
        IMemPool *mBufferPool = nullptr;
        std::deque<memPoolSlice *> mSliceQueue;
        int64_t mCapacity;
        int mSliceSize;
        mutex mSliceLock;
    };
}


#endif //FRAMEWORK_ISLICEMANAGER_H
