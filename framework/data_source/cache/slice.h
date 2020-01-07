//
// Created by moqi on 2018/12/17.
//

#ifndef CICADA_PLAYER_SLICE_H
#define CICADA_PLAYER_SLICE_H

#include <stdint.h>
#include <mutex>

namespace Cicada{
    // TODO: add get mBufferPtr to read write directly

    class ISliceManager;

    class slice {
    public:
        slice(uint64_t capacity, uint64_t position, uint8_t *bufferPtr);

        slice(uint64_t capacity, uint64_t position);

        virtual ~slice();

        virtual int write(const void *buffer, int size);

        virtual int readAt(void *buffer, int size, uint64_t offset);

        uint64_t getPosition()
        {
            return mPosition;
        }

        uint64_t getValidSize()
        {
            return mSize;
        }

        uint64_t getRemainSize()
        {
            return mCapacity - mSize;
        }

        void lock()
        {
            mMutex.lock();
        }

        void unlock()
        {
            mMutex.unlock();
        }

    private:
        slice(const slice&);
        slice& operator=(const slice&);

    protected:
        uint64_t mSize = 0;
        uint64_t mCapacity = 0;
        uint64_t mPosition = 0;
        uint8_t *mBufferPtr = nullptr;
        bool     mAlloc = false;
        std::mutex mMutex;
    };

    class SliceReleaseCb {
    public:
        virtual bool onReleaseReferenceSlice(slice *slice) = 0;
    };
}


#endif //CICADA_PLAYER_SLICE_H
