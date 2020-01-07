//
// Created by moqi on 2019-01-04.
//

#include "memPoolSlice.h"
#include <cassert>

namespace Cicada {
    memPoolSlice::memPoolSlice(uint64_t capacity, uint64_t position, uint8_t *bufferPtr, SliceReleaseCb &release)
        : slice(capacity, position, bufferPtr)
        , mReleaseCb(release)
    {
    }

    uint8_t *memPoolSlice::getBuffer()
    {
        return mBufferPtr;
    }

    memPoolSlice::~memPoolSlice()
    {
        assert(mBufferPtr != nullptr);
        mBufferPtr = nullptr;
    }

    bool memPoolSlice::tryReleaseReference()
    {
        bool ret = mReleaseCb.onReleaseReferenceSlice(this);
        return ret;
    }

    int memPoolSlice::write(const void *buffer, int size)
    {
        // TODO: if (mBufferPtr == null)
        // TODO: mark first write time
        assert(mBufferPtr != nullptr);
        return slice::write(buffer, size);
    }

    int memPoolSlice::readAt(void *buffer, int size, uint64_t offset)
    {
        return slice::readAt(buffer, size, offset);
    }

}
