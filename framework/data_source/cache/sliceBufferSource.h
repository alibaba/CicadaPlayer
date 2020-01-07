//
// Created by moqi on 2019-01-04.
//

#ifndef PRIVATESERVICE_SLICEBUFFERSOURCE_H
#define PRIVATESERVICE_SLICEBUFFERSOURCE_H

#include <cstdint>
#include "slice.h"
#include "ISliceManager.h"

namespace Cicada{
    class sliceBuffer : public SliceReleaseCb {
    public:
        sliceBuffer(uint64_t sliceSize,uint64_t maxUsedBufferSize,uint64_t capacity, ISliceManager *manager);

        virtual ~sliceBuffer();

        virtual int writeAt(const uint8_t *buffer, int size, uint64_t offset);

        virtual int readAt(uint8_t *buffer, int size, uint64_t offset);

        bool onReleaseReferenceSlice(slice *slice) override;

        void dump();

        int getSliceGotCount()
        {
            return mSliceCountGot;
        }

    protected:
        slice **mSlices;
        uint64_t mSliceSize; //the mem size of each slice
        uint64_t mCapacity;
        uint32_t mSliceCount;// the count of slice to spilt the source
        ISliceManager *mManager = nullptr;
        std::recursive_mutex mMutex;
        int mSliceCountGot = 0;// the count of slice got from SliceManager or malloced
        int mMaxReadSliceCount = 100;
    };

    class sliceBufferSource : public sliceBuffer {
    public:
        class sliceBufferSourceCallBack {
        public:
            virtual int onReadSource(uint8_t *buffer, int size, uint64_t pos) = 0;
        };

        sliceBufferSource(uint64_t sliceSize,uint64_t maxUsedBufferSize, uint64_t capacity, sliceBufferSource::sliceBufferSourceCallBack &sourceCb,
                          ISliceManager *sliceManager);

        ~sliceBufferSource() override;

        int readAt(uint8_t *buffer, int size, uint64_t offset) override;

    private:
        int getSliceFromSource(uint8_t *buffer, uint64_t sliceNum);

    private:
        sliceBufferSourceCallBack &mSourceCallBack;

    };
}


#endif //PRIVATESERVICE_SLICEBUFFERSOURCE_H
