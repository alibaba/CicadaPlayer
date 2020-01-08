//
// Created by moqi on 2019-01-04.
//

#include <cinttypes>
#include <utils/frame_work_log.h>
#include <cassert>
#include <cstring>
#include "sliceBufferSource.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

namespace Cicada {
#define ADD_LOCK std::unique_lock<std::recursive_mutex> uMutex(mMutex)

    sliceBuffer::sliceBuffer(uint64_t sliceSize, uint64_t maxUsedBufferSize, uint64_t capacity, ISliceManager *manager)
    {
        mSliceSize = sliceSize;
        mCapacity = capacity;
        mMaxReadSliceCount = (int)maxUsedBufferSize / mSliceSize;
        mSliceCount = static_cast<uint32_t>(mCapacity / mSliceSize);

        if (mCapacity % mSliceSize) {
            mSliceCount++;
        }

        mSlices = new slice *[mSliceCount];
        memset(mSlices, 0, sizeof(slice *) * mSliceCount);
        mManager = manager;
        mSliceCountGot = 0;
//        for (int i = 0; i < mSliceCount; ++i) {
//            if (manager)
//                mSlices[i] = manager->getSlice(capacity, i * mSliceSize);
//            else
//                mSlices[i] = new slice(mSliceSize, i * mSliceSize);
//        }
    }

    sliceBuffer::~sliceBuffer()
    {
        for (int i = 0; i < mSliceCount; ++i) {
            if (!mManager) {
                delete mSlices[i];
            } else {
                mManager->returnSlice(mSlices[i]);
            }
        }

        delete[] mSlices;
    }


// TODO: use writeAt

    int sliceBuffer::writeAt(const uint8_t *buffer, int size, uint64_t offset)
    {
//        AF_LOGD("write size is %d write pos is %"
//                        PRIu64
//                        "\n", size, offset);
        int curNum = static_cast<int>(offset / mSliceSize);
        uint64_t startPos = offset % mSliceSize;

        if (startPos > 0) {
            curNum++;
            startPos = mSliceSize * curNum - offset;
        }

//        AF_LOGD("write %d slice skip %d data\n", curNum, startPos);
//        AF_LOGD("mSliceCount is %" PRIu32"\n", mSliceCount);
        bool isLastSlice = curNum == mSliceCount - 1;

        if (size - startPos < mSliceSize && !isLastSlice) {
            return 0;
        }

        int beforePos = static_cast<int>(startPos);
        size -= startPos;

        for (int i = curNum; i < mSliceCount; ++i) {
            ADD_LOCK;

            if (mSlices[i] == nullptr) {
                if (mManager) {
                    mSlices[i] = mManager->getSlice(mSliceSize, i * mSliceSize, *this);
                } else {
                    mSlices[i] = new slice(mSliceSize, i * mSliceSize);
                }

                if (nullptr != mSlices[i]) {
                    mSliceCountGot++;
                }
            }

            isLastSlice = curNum == mSliceCount - 1;
            int writeSize = (int) MIN(mSliceSize, size);

            if (size - startPos < mSliceSize && !isLastSlice) {
                break;
            }

            if ((nullptr != mSlices[i])
                    && (0 < mSlices[i]->getRemainSize())) {
                writeSize = mSlices[i]->write(buffer + startPos, writeSize);
            } else {
                AF_LOGI("slice %d is filed\n", i);
            }

            startPos += writeSize;
            size -= writeSize;

            if (size <= 0) {
                break;
            }
        }

        return static_cast<int>(startPos - beforePos);
    }

    int sliceBuffer::readAt(uint8_t *buffer, int size, uint64_t offset)
    {
        uint64_t curNum = offset / mSliceSize;
        uint64_t startPos = offset % mSliceSize;

        if (curNum >= mSliceCount) {
            return 0;
        }

        if (mSlices[curNum] == nullptr) {
            return -EAGAIN;
        }

        int sizeRead = 0;

        for (uint64_t i = curNum; i < mSliceCount; ++i) {
            if (mSlices[i]->getValidSize() == 0) {
                break;
            }

            sizeRead += mSlices[i]->readAt(buffer + sizeRead, static_cast<int>(mSliceSize), startPos);
            startPos = 0;

            if (sizeRead >= size) {
                break;
            }
        }

        return sizeRead;
    }

    void sliceBuffer::dump()
    {
        int size = 0;

        for (int i = 0; i < mSliceCount; ++i) {
            //         AF_LOGD("%d slice size is %d", i, mSlices[i]->getSize());
            ADD_LOCK;

            if (mSlices[i]) {
                size += mSlices[i]->getValidSize();
            }
        }

        AF_LOGD("sliceBuffer size is %d\n", size);
    }

    bool sliceBuffer::onReleaseReferenceSlice(slice *slice)
    {
        uint64_t i = slice->getPosition() / mSliceSize;
        //AF_TRACE;
        assert(i < mSliceCount);

        if (i < mSliceCount) {
            ADD_LOCK;
            assert(mSlices[i] != nullptr);
            mSlices[i] = nullptr;
            mSliceCountGot--;
            assert(mSliceCountGot >= 0);
            return true;
        }

        return false;
    }

    sliceBufferSource::sliceBufferSource(uint64_t
                                         sliceSize, uint64_t maxUsedBufferSize, uint64_t
                                         capacity, sliceBufferSource::sliceBufferSourceCallBack &sourceCb,
                                         ISliceManager *sliceManager)
        : sliceBuffer(sliceSize, maxUsedBufferSize, capacity, sliceManager),
          mSourceCallBack(sourceCb)
    {
    }

    sliceBufferSource::~sliceBufferSource()
    {
    }

    int sliceBufferSource::getSliceFromSource(uint8_t *buffer, uint64_t sliceNum)
    {
        AF_LOGD("%s get slice %llu\n", __func__, sliceNum);
        uint64_t pos = sliceNum * mSliceSize;
        int ret = mSourceCallBack.onReadSource(buffer, mSliceSize, pos);

        if (ret <= 0) {
            return ret;
        }

        int writeSize = writeAt(buffer, ret, pos);
        assert(writeSize == ret);
        return ret;
    }

    int sliceBufferSource::readAt(uint8_t *buffer, int size, uint64_t offset)
    {
        uint64_t curNum = offset / mSliceSize;
        uint64_t startPos = offset % mSliceSize;
        uint8_t *readBuffer = nullptr;
        int ret = 0;
//        AF_LOGD("readAt: size %d,where %lld", size, offset);

        if (curNum >= mSliceCount) {
            return 0;
        }

        if (startPos > 0) {
//            AF_LOGD("seek slice %d to %d\n", curNum, startPos);
        }

        int sizeBufferFilled = 0;

        for (uint64_t i = curNum; i < mSliceCount; ++i) {
            if (mSlices[i] == nullptr) {
                if (mSliceCountGot > mMaxReadSliceCount) {
                    return mSourceCallBack.onReadSource(buffer, size, offset);
                }

                if (readBuffer == nullptr) {
                    readBuffer = new uint8_t[mSliceSize];
                }

                ret = getSliceFromSource(readBuffer, i);

                if (ret <= 0) {
                    AF_LOGE("getSliceFromSource error %d\n", ret);
                    delete [] readBuffer;
                    return ret;
                }
            }

            if (nullptr != buffer) {
                ADD_LOCK;

                if (mSlices[i] != nullptr) {
                    if (mManager) {
                        mManager->updateSliceUseTime(mSlices[i]);
                    }

                    ret = mSlices[i]->readAt(buffer + sizeBufferFilled, size, startPos);
                } else {
                    memcpy(buffer + sizeBufferFilled, readBuffer + startPos, size);
                    AF_TRACE;
                }
            }

            startPos = 0;
            sizeBufferFilled += ret;
            size -= ret;
//            AF_LOGD("read %d slice %d\n", i, ret);

            if (size <= 0) {
                break;
            }
        }

        delete [] readBuffer;
//        AF_LOGD("sizeRead is %d\n", sizeBufferFilled);
        return sizeBufferFilled;
    }
}
