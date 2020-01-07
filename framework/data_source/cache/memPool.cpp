//
// Created by moqi on 2019-01-02.
//

#include "memPool.h"

namespace Cicada {

    fixSizePool::fixSizePool(int sliceSize, uint64_t capacity)
    {
        mBufferSize = sliceSize;
        mBufferNum = capacity / mBufferSize;
        mAllocedCount = 0;
    }

    fixSizePool::~fixSizePool()
    {
        std::lock_guard<std::mutex> lock(mMutex);

        while (!mBufferQueue.empty()) {
            delete mBufferQueue.front();
            mBufferQueue.pop_front();
        }
    }

    uint8_t *fixSizePool::getBuffer()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        uint8_t *buffer = nullptr;

        if (!mBufferQueue.empty()) {
            buffer = mBufferQueue.front();
            mBufferQueue.pop_front();
            return buffer;
        }

        if (mAllocedCount >= mBufferNum) {
            return nullptr;
        }

        buffer = new uint8_t[mBufferSize];
        mAllocedCount++;
        return buffer;
    }

    void fixSizePool::releaseBuffer(uint8_t *buffer)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mBufferQueue.push_back(buffer);
    }
}
