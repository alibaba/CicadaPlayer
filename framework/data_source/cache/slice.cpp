//
// Created by moqi on 2018/12/17.
//

#define LOG_TAG "sliceBuffer"

#include <cassert>

#ifndef WIN32

    #include <sys/param.h>

#endif

#include <cerrno>
#include <cstring>
#include <utils/frame_work_log.h>
#include <cstdio>
#include <cinttypes>
#include "slice.h"
#include "ISliceManager.h"

#ifndef MIN
    #define MIN(a,b) (((a)<(b))?(a):(b))
    #define MAX(a,b) (((a)>(b))?(a):(b))
#endif

namespace Cicada {
    slice::slice(uint64_t capacity, uint64_t position)
        : slice(capacity, position, nullptr)
    {
    }

    slice::slice(uint64_t capacity, uint64_t position, uint8_t *bufferPtr)
        : mCapacity(capacity)
        , mPosition(position)
        , mBufferPtr(bufferPtr)
    {
    }

    slice::~slice()
    {
        if (mAlloc) {
            delete[] mBufferPtr;
        }
    }

    int slice::write(const void *buffer, int size)
    {
        if (mBufferPtr == nullptr) {
            mAlloc = true;
            mBufferPtr = new uint8_t[mCapacity];
        }

        if (size == 0) {
            return 0;
        }

        int writeSize = (int) MIN(size, mCapacity - mSize);

        if (writeSize == 0) {
            return -ENOSPC;
        }

        memcpy(mBufferPtr + mSize, buffer, static_cast<size_t>(writeSize));
        mSize += writeSize;
        return writeSize;
    }

    int slice::readAt(void *buffer, int size, uint64_t offset)
    {
        int readSize = (int) MIN(size, (int) (mSize - offset));

        if (readSize > 0 && buffer != nullptr) {
            memcpy(buffer, mBufferPtr + offset, static_cast<size_t>(readSize));
        }

        return readSize;
    }
}
