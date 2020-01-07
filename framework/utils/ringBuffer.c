
/*
 *      Copyright (C) 2015 pingkai010@gmail.com
 *
 */

// add lock
#include "ringBuffer.h"
#include <stdlib.h>
#include <string.h>

typedef struct RingBuffer_t {
    char *m_buffer;
    unsigned int m_size;
    unsigned int m_readPtr;
    unsigned int m_writePtr;
    unsigned int m_fillCount;
} RingBuffer;

RingBuffer *RingBufferCreate(uint32_t size)
{
    RingBuffer *rBuf;
    char *buf = malloc(size);
    if (!buf)
        return NULL;
    rBuf = malloc(sizeof(RingBuffer));
    memset(rBuf, 0, sizeof(RingBuffer));
    rBuf->m_size = size;
    rBuf->m_buffer = buf;
    return rBuf;
}

void RingBufferDestroy(RingBuffer *rBuf)
{
    if (rBuf->m_buffer != NULL) {
        free(rBuf->m_buffer);
        rBuf->m_buffer = NULL;
    }
    free(rBuf);
    return;
}

void RingBufferClear(RingBuffer *rBuf)
{
    rBuf->m_readPtr = 0;
    rBuf->m_writePtr = 0;
    rBuf->m_fillCount = 0;
    return;
}

uint32_t RingBufferReadData(RingBuffer *rBuf, char *buf, uint32_t size)
{
    if (size > rBuf->m_fillCount) {
        return 0;
    }
    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, chunk);
        memcpy(buf + chunk, rBuf->m_buffer, size - chunk);
        rBuf->m_readPtr = size - chunk;
    } else {
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, size);
        rBuf->m_readPtr += size;
    }
    if (rBuf->m_readPtr == rBuf->m_size)
        rBuf->m_readPtr = 0;
    rBuf->m_fillCount -= size;
    return size;
}

uint32_t RingBufferWriteData(RingBuffer *rBuf, const char *buf, uint32_t size)
{
    if (size > rBuf->m_size - rBuf->m_fillCount) {
        return 0;
    }
    if (size + rBuf->m_writePtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_writePtr;
        memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, chunk);
        memcpy(rBuf->m_buffer, buf + chunk, size - chunk);
        rBuf->m_writePtr = size - chunk;
    } else {
        if ((rBuf->m_buffer + rBuf->m_writePtr) != buf)
            memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, size);
        rBuf->m_writePtr += size;
    }
    if (rBuf->m_writePtr == rBuf->m_size)
        rBuf->m_writePtr = 0;
    rBuf->m_fillCount += size;
    return size;
}

int32_t RingBufferSkipBytes(RingBuffer *rBuf, int skipSize)
{
    if (skipSize < 0) {
        return 0; // skipping backwards is not supported
    }
    int size = skipSize;
    if (size > rBuf->m_fillCount) {
        return 0;
    }
    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        rBuf->m_readPtr = size - chunk;
    } else {
        rBuf->m_readPtr += size;
    }
    if (rBuf->m_readPtr == rBuf->m_size)
        rBuf->m_readPtr = 0;
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
    return rBuf->m_fillCount;
}

uint32_t RingBuffergetMaxWriteSize(RingBuffer *rBuf)
{
    return rBuf->m_size - rBuf->m_fillCount;
}