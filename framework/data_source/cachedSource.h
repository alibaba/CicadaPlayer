//
// Created by moqi on 2018-12-17.
//

#ifndef CICADA_PLAYER_CACHEDSOURCE_H
#define CICADA_PLAYER_CACHEDSOURCE_H

#include <mutex>
#include <data_source/cache/ISliceManager.h>
#include <data_source/cache/sliceBufferSource.h>
#include "data_source/cache/slice.h"
#include "IDataSource.h"

namespace Cicada{
    class cachedSource : private sliceBufferSource::sliceBufferSourceCallBack {
    public:
        explicit cachedSource(const string &url,uint64_t maxUsedBufferSize);

        void setSliceManager(ISliceManager *manager);

        virtual ~cachedSource();

        int Open(int flags);

        void Close();

        void  Interrupt(bool bInterrput);

        int readAt(void *buf, size_t nbyte, uint64_t where);

        int64_t getFileSize()
        {
            return mFileSize;
        }

        IDataSource *getDataSource()
        {
            return mDataSource;
        }

        int getSliceGotCount()
        {
            if(mBufferSource){
                return mBufferSource->getSliceGotCount();
            }
            return 0;
        }

    private:

        int onReadSource(uint8_t *buffer, int size, uint64_t pos) override;

    private:
        sliceBufferSource *mBufferSource = nullptr;
        IDataSource *mDataSource = nullptr;
        int64_t mFileSize = -1;
        std::mutex mMutex;
        ISliceManager *mSliceManager = nullptr;
        uint64_t mMaxUsedBufferSize;
        bool mIsOpen = false;
    };
}


#endif //CICADA_PLAYER_CACHEDSOURCE_H
