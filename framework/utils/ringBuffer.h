
/*
 *      Copyright (C) 2015 pingkai010@gmail.com
 *
 */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct RingBuffer_t RingBuffer;

RingBuffer *RingBufferCreate(uint32_t size);

void RingBufferSetBackSize(RingBuffer *rBuf,uint32_t size);

void RingBufferDestroy(RingBuffer *rBuf);

void RingBufferClear(RingBuffer *rBuf);

uint32_t RingBufferReadData(RingBuffer *rBuf, char *buf, uint32_t size);

uint32_t RingBufferWriteData(RingBuffer *rBuf, const char *buf, uint32_t size);

int32_t RingBufferSkipBytes(RingBuffer *rBuf, int skipSize);

char *getBuffer(RingBuffer *rBuf);

uint32_t RingBuffergetSize(RingBuffer *rBuf);

unsigned int RingBuffergetReadPtr(RingBuffer *rBuf);

unsigned int RingBuffergetWritePtr(RingBuffer *rBuf);

uint32_t RingBuffergetMaxReadSize(RingBuffer *rBuf);

uint32_t RingBuffergetMaxWriteSize(RingBuffer *rBuf);
uint32_t RingBuffergetMaxBackSize(RingBuffer *rBuf);

#ifdef __cplusplus
}
#endif
#endif

