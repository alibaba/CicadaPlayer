//
// Created by moqi on 2019-01-02.
//
#define LOG_TAG "ISliceManager"

#include <utils/frame_work_log.h>
#include "ISliceManager.h"
#include <utils/property.h>
#include <string>
#include <cassert>
#include <mutex>
using namespace std;

#ifdef __ANDROID__
    //add for  atoll function.
    #include <cstdlib>
#endif

#define SLICE_SIZE 1024*32

namespace Cicada {
    static ISliceManager *g_manager = nullptr;

    ISliceManager *ISliceManager::getManager()
    {
        static std::once_flag oc;
        std::call_once(oc, [&] {
            g_manager = new ISliceManager();
        });
        return g_manager;
    }

//TODO: set the pool size
    ISliceManager::ISliceManager()
    {
        mCapacity = atoll(getProperty("SliceManager.capacityM"));

        if (mCapacity > 0) {
            mCapacity *= 1024 * 1024;
        } else {
            mCapacity = 100 * 1024 * 1024;
        }

        //mCapacity = 6 * 1024 * 1024;
        mSliceSize = atoi(getProperty("ro.SliceManager.sliceSizeK"));

        if (mSliceSize > 0) {
            mSliceSize *= 1024;
        } else {
            mSliceSize = SLICE_SIZE;
        }

        mBufferPool = new fixSizePool(mSliceSize, static_cast<uint64_t>(mCapacity));
    }

    ISliceManager::~ISliceManager()
    {
        for (auto item = mSliceQueue.begin(); item != mSliceQueue.end();) {
            memPoolSlice *slice = *item;
            mBufferPool->releaseBuffer(slice->getBuffer());
            delete slice;
            item = mSliceQueue.erase(item);
        }

        delete mBufferPool;
    }

    slice *ISliceManager::getSlice(uint64_t capacity, uint64_t position, SliceReleaseCb &release)
    {
        uint8_t *buffer = mBufferPool->getBuffer();

        if (buffer == nullptr) {
            //TODO: cal the slice score,release the lowest one
            memPoolSlice *deleteSlice = nullptr;
            {
                std::unique_lock<std::mutex> lock(mSliceLock);

                if (mSliceQueue.empty()) {
                    return nullptr;
                }

                deleteSlice = mSliceQueue.front();
                mSliceQueue.pop_front();
            }

            // don't lock mSliceLock when tryReleaseReference, which maybe lead to ANR
            if (deleteSlice->tryReleaseReference()) {
                buffer = deleteSlice->getBuffer();
                delete deleteSlice;
            } else {
                // Todo more, tryReleaseReference always success at present
                std::unique_lock<std::mutex> lock(mSliceLock);
                mSliceQueue.push_front(deleteSlice);
                return nullptr;
            }
        }

        std::unique_lock<std::mutex> lock(mSliceLock);
        memPoolSlice *slice = new memPoolSlice(capacity, position, buffer, release);
        mSliceQueue.push_back(slice);
        return slice;
    }

    int ISliceManager::getSliceSize()
    {
        return mSliceSize;
    }

    void ISliceManager::updateSliceUseTime(slice *pSlice)
    {
        std::unique_lock<std::mutex> lock(mSliceLock);
        memPoolSlice *slice = nullptr;

        for (auto item = mSliceQueue.begin(); item != mSliceQueue.end();) {
            if (*item == pSlice) {
                slice = *item;
                item = mSliceQueue.erase(item);
                break;
            } else {
                item++;
            }
        }

        if (slice) {
            mSliceQueue.push_back(slice);
        }
    }

    void ISliceManager::returnSlice(slice *pSlice)
    {
        std::unique_lock<std::mutex> lock(mSliceLock);

        for (auto item = mSliceQueue.begin(); item != mSliceQueue.end();) {
            //TODO: cal the slice score,release the lowest one
            if (*item == pSlice) {
                memPoolSlice *slice = *item;
                mBufferPool->releaseBuffer(slice->getBuffer());
                item = mSliceQueue.erase(item);
                delete slice;
                break;
            } else {
                item++;
            }
        }
    }
}
