//
// Created by moqi on 2018-12-17.
//

#include <utils/frame_work_log.h>
#include <cerrno>
#include <data_source/cachedSource.h>

#include "data_source/cachedSource.h"
#include "data_source/dataSourcePrototype.h"

namespace Cicada {

    cachedSource::cachedSource(const string &url, uint64_t maxUsedBufferSize)
    {
        mMaxUsedBufferSize = maxUsedBufferSize;
        mDataSource = dataSourcePrototype::create(url);
    }

    int cachedSource::onReadSource(uint8_t *buffer, int size, uint64_t pos)
    {
        int sizeRead = 0;
        int64_t seekPos = mDataSource->Seek(pos, SEEK_SET);

        if (seekPos < 0) {
            return seekPos;
        }

        while (size > 0) {
            int ret;
            ret = mDataSource->Read(buffer + sizeRead, static_cast<size_t>(size));

            if (ret < 0) {
                return ret;
            }

            if (ret == 0) {
                break;
            }

            size -= ret;
            sizeRead += ret;
        }

        return sizeRead;
    }

    cachedSource::~cachedSource()
    {
        if (mDataSource) {
            mDataSource->Close();
            delete mDataSource;
        }

        delete mBufferSource;
    }

    int cachedSource::Open(int flags)
    {
        std::lock_guard<std::mutex> uMutex(mMutex);

        if (mIsOpen) {
            return 0;
        }

        IDataSource::SourceConfig config{};
        config.low_speed_time_ms = 15000;
        config.connect_time_out_ms = 15000;
        mDataSource->Set_config(config);
        int ret = mDataSource->Open(flags);

        if (ret < 0) {
            return ret;
        }

        mFileSize = mDataSource->Seek(0, SEEK_SIZE);

        if (mFileSize <= 0) {
            AF_LOGE("unknown file size can't cache");
            return -1;
        }

        mIsOpen = true;

        if (mMaxUsedBufferSize == 0) {
            mMaxUsedBufferSize = mFileSize;
        }

//        std::lock_guard<std::mutex> uMutex(mMutex);
        if (mBufferSource) {
            delete mBufferSource;
        }

        mBufferSource = new sliceBufferSource((uint64_t) ISliceManager::getManager()->getSliceSize(), mMaxUsedBufferSize, (uint64_t) mFileSize,
                                              *this, mSliceManager);
        return 0;
    }

    void cachedSource::Interrupt(bool bInterrput)
    {
        mDataSource->Interrupt(bInterrput);
    }

    void cachedSource::Close()
    {
        std::lock_guard<std::mutex> uMutex(mMutex);
        mIsOpen = false;
        mDataSource->Close();
    }

    int cachedSource::readAt(void *buf, size_t nbyte, uint64_t where)
    {
        if (mIsOpen == false) {
            int ret = Open(0);

            if (ret < 0) {
                return ret;
            }
        }

        std::lock_guard<std::mutex> uMutex(mMutex);

        if (mBufferSource && mIsOpen) {
            return mBufferSource->readAt(static_cast<uint8_t *>(buf), nbyte, where);
        } else {
            AF_LOGI("read error :%p mBufferSource:%p", this, mBufferSource);
        }

        return -1;
    }

    void cachedSource::setSliceManager(ISliceManager *manager)
    {
        mSliceManager = manager;
    }
}
