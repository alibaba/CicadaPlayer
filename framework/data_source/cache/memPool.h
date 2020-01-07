//
// Created by moqi on 2019-01-02.
//

#ifndef FRAMEWORK_MEMPOOL_H
#define FRAMEWORK_MEMPOOL_H

#include <cstdint>
#include <deque>
#include <mutex>

namespace Cicada{
    class IMemPool {
    public:
        IMemPool() = default;

        virtual ~IMemPool() = default;

        virtual uint8_t *getBuffer() = 0;

        virtual void releaseBuffer(uint8_t *buffer) = 0;


    };

    class fixSizePool : public IMemPool {
    public:
        fixSizePool(int sliceSize, uint64_t capacity);

        ~fixSizePool() override;

        uint8_t *getBuffer() override;

        void releaseBuffer(uint8_t *buffer) override;

    private:
        int mBufferSize;
        uint64_t mBufferNum;
        uint64_t mAllocedCount;
        std::deque<uint8_t *> mBufferQueue;
        std::mutex mMutex;
    };
}


#endif //FRAMEWORK_MEMPOOL_H
