
/*
 *      Copyright (C) 2015 pingkai010@gmail.com
 *
 */

// add lock
#include "ringBuffer.h"
#include "frame_work_log.h"
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <algorithm>
#include <cassert>

using namespace std;

typedef struct RingBuffer_t {
    char *m_buffer;
    unsigned int m_size;
    unsigned int m_readPtr;
    unsigned int m_writePtr;
    atomic<uint32_t> m_fillCount;
    unsigned int m_back_size;
    atomic<uint32_t> m_backCount;
} RingBuffer;

RingBuffer *RingBufferCreate(uint32_t size)
{
    RingBuffer *rBuf;
    char *buf = (char *) malloc(size);

    if (!buf) {
        return nullptr;
    }

    rBuf = (RingBuffer *) malloc(sizeof(RingBuffer));
    memset(rBuf, 0, sizeof(RingBuffer));
    rBuf->m_size = size;
    rBuf->m_buffer = buf;
    rBuf->m_back_size = 0;
    rBuf->m_backCount = 0;
    return rBuf;
}

void RingBufferSetBackSize(RingBuffer *rBuf, uint32_t size)
{
    rBuf->m_back_size = size;
}

void RingBufferDestroy(RingBuffer *rBuf)
{
    if (rBuf->m_buffer != nullptr) {
        free(rBuf->m_buffer);
        rBuf->m_buffer = nullptr;
    }

    free(rBuf);
}

void RingBufferClear(RingBuffer *rBuf)
{
    rBuf->m_readPtr = 0;
    rBuf->m_writePtr = 0;
    rBuf->m_fillCount = 0;
    rBuf->m_backCount = 0;
}

uint32_t RingBufferReadData(RingBuffer *rBuf, char *buf, uint32_t size)
{
    if (size > rBuf->m_fillCount.load()) {
        return 0;
    }

    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, chunk);
        memcpy(buf + chunk, rBuf->m_buffer, size - chunk);
        rBuf->m_readPtr = size - chunk;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    } else {
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, size);
        rBuf->m_readPtr += size;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    }

    if (rBuf->m_readPtr == rBuf->m_size) {
        rBuf->m_readPtr = 0;
    }

    rBuf->m_fillCount -= size;
    rBuf->m_backCount = std::min(rBuf->m_back_size, rBuf->m_backCount + size);
    return size;
}

uint32_t RingBufferWriteData(RingBuffer *rBuf, const char *buf, uint32_t size)
{
    if (size > rBuf->m_size - rBuf->m_fillCount.load() - rBuf->m_backCount.load()) {
        return 0;
    }

    if (size + rBuf->m_writePtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_writePtr;
        memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, chunk);
        memcpy(rBuf->m_buffer, buf + chunk, size - chunk);
        rBuf->m_writePtr = size - chunk;
    } else {
        if ((rBuf->m_buffer + rBuf->m_writePtr) != buf) {
            memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, size);
        }

        rBuf->m_writePtr += size;
    }

    if (rBuf->m_writePtr == rBuf->m_size) {
        rBuf->m_writePtr = 0;
    }

    rBuf->m_fillCount += size;
    return size;
}

int32_t RingBufferSkipBytes(RingBuffer *rBuf, int skipSize)
{
    int size;

    if (skipSize < 0) {
        size = -skipSize;
        //    AF_LOGI("skip size is %d\n", skipSize);

        if (rBuf->m_backCount.load() >= size) {
            rBuf->m_backCount -= size;

            if (rBuf->m_readPtr < size) {
                rBuf->m_readPtr = rBuf->m_size - (size - rBuf->m_readPtr);
                assert(rBuf->m_readPtr <= rBuf->m_size);
            } else {
                rBuf->m_readPtr -= size;
                assert(rBuf->m_readPtr <= rBuf->m_size);
            }

            if (rBuf->m_readPtr == rBuf->m_size) {
                rBuf->m_readPtr = 0;
            }

            rBuf->m_fillCount += size;
            return skipSize;
        }

        return 0;
    }

    size = skipSize;

    if (size > rBuf->m_fillCount) {
        return 0;
    }

    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        rBuf->m_readPtr = size - chunk;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    } else {
        rBuf->m_readPtr += size;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    }

    if (rBuf->m_readPtr == rBuf->m_size) {
        rBuf->m_readPtr = 0;
    }

    rBuf->m_fillCount -= size;
    return skipSize;
}

char *getBuffer(RingBuffer *rBuf)
{
    return rBuf->m_buffer;
}

uint32_t RingBuffergetSize(RingBuffer *rBuf)
{
    return rBuf->m_size;
}

unsigned int RingBuffergetReadPtr(RingBuffer *rBuf)
{
    return rBuf->m_readPtr;
}

unsigned int RingBuffergetWritePtr(RingBuffer *rBuf)
{
    return rBuf->m_writePtr;
}

uint32_t RingBuffergetMaxReadSize(RingBuffer *rBuf)
{
    return rBuf->m_fillCount.load();
}

uint32_t RingBuffergetMaxWriteSize(RingBuffer *rBuf)
{
    return rBuf->m_size - rBuf->m_fillCount.load() - rBuf->m_backCount.load();
}

uint32_t RingBuffergetMaxBackSize(RingBuffer *rBuf)
{
    return rBuf->m_backCount;
}