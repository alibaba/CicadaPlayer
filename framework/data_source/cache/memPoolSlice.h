//
// Created by moqi on 2019-01-04.
//

#ifndef PRIVATESERVICE_MEMPOOLSLICE_H
#define PRIVATESERVICE_MEMPOOLSLICE_H

#include "slice.h"
#include <cstdint>

namespace Cicada{
    class memPoolSlice : public slice {
    public:
        memPoolSlice(uint64_t capacity, uint64_t position, uint8_t *bufferPtr, SliceReleaseCb &release);

        ~memPoolSlice() override;

        uint8_t *getBuffer();

        int write(const void *buffer, int size) override;

        int readAt(void *buffer, int size, uint64_t offset) override;

        bool tryReleaseReference();
    private:
        SliceReleaseCb &mReleaseCb;
    };

}


#endif //PRIVATESERVICE_MEMPOOLSLICE_H
