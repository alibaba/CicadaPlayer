//
// Created by lifujun on 2019/5/27.
//

#define LOG_TAG "CacheFileRemuxer"

#include "CacheFileRemuxer.h"
#include "CacheRet.h"

#include <utils/file/FileUtils.h>
#include <utility>
#include <muxer/ffmpegMuxer/FfmpegMuxer.h>
#include <muxer/IMuxerPrototype.h>
#include <utils/frame_work_log.h>
#include <utils/stringUtil.h>
#include <utils/mediaFrame.h>

using namespace Cicada;

CacheFileRemuxer::CacheFileRemuxer(const string &destFilePath, const string &description)
{
    mDestFilePath = destFilePath;
    mDescription = description;
    AF_LOGD("mDestFilePath = %s", mDestFilePath.c_str());
}

CacheFileRemuxer::~CacheFileRemuxer()
{
    stop();

    if (mMuxer != nullptr) {
        delete mMuxer;
        mMuxer = nullptr;
    }

    if (mDestFileCntl != nullptr) {
        delete mDestFileCntl;
        mDestFileCntl = nullptr;
    }

    mFrameInfoQueue.clear();
}

void CacheFileRemuxer::addFrame(const unique_ptr<IAFPacket> &frame, StreamType type)
{
    if (frame == nullptr) {
        mFrameEof = true;
    } else {
        mFrameEof = false;

        FrameInfo *info = new FrameInfo();
        info->frame = frame->clone();
        info->type = type;
        {
            std::unique_lock<mutex> lock(mQueueMutex);
            mFrameInfoQueue.push_back(std::unique_ptr<FrameInfo>(info));
            mQueueCondition.notify_one();
        }
    }
}


bool CacheFileRemuxer::prepare()
{
    if (FileUtils::isFileExist(mDestFilePath.c_str()) == FILE_TRUE) {
        //???
        if (FileUtils::rmrf(mDestFilePath.c_str()) != FILE_TRUE) {
            return false;
        }
    }

    if (FileUtils::touch(mDestFilePath.c_str()) != FILE_TRUE) {
        return false;
    }

    return true;
}

void CacheFileRemuxer::start()
{
    AF_LOGD("CacheFileRemuxer::start...");
    stop();
    {
        std::unique_lock<mutex> lock(mThreadMutex);
        mWantStop = false;
        mMuxThread = NEW_AF_THREAD(muxThreadRun);
        mMuxThread->start();
    }
}


int CacheFileRemuxer::muxThreadRun()
{
    {
        std::unique_lock<mutex> lock(mThreadMutex);

        if (mInterrupt || mWantStop) {
            AF_LOGW("muxThreadRun() mInterrupt || mWantStop...");
            return -1;
        }
    }
    AF_LOGD("muxThreadRun() start...");
    {
        std::unique_lock<mutex> lock(mObjectMutex);

        if (mMuxer != nullptr) {
            mMuxer->close();
            delete mMuxer;
            mMuxer = nullptr;
        }

        if (mDestFileCntl != nullptr) {
            delete mDestFileCntl;
            mDestFileCntl = nullptr;
        }

        mMuxer = IMuxerPrototype::create(mDestFilePath, "mp4", mDescription);
        mDestFileCntl = new FileCntl(mDestFilePath);
    }

    if (mMuxer == nullptr) {
        sendError(CACHE_ERROR_ENCRYPT_CHECK_FAIL);
        return -1;
    }

    initMuxer();
    int openRet = mMuxer->open();

    if (openRet != 0) {
        AF_LOGE("muxThreadRun() mMuxer->open() fail...ret = %d ", openRet);
        //open fail..
        sendError(CACHE_ERROR_MUXER_OPEN);
        return -1;
    }

    bool hasError = false;

    while (true) {
        {
            std::unique_lock<mutex> lock(mQueueMutex);

            if (mFrameInfoQueue.empty()) {

                if (mFrameEof) {
                    AF_LOGW("muxThreadRun() mFrameEof...");
                    break;
                }

                mQueueCondition.wait_for(lock, std::chrono::milliseconds(10),
                                         [this]() { return this->mInterrupt || this->mWantStop || this->mFrameEof; });

            } else {
                unique_ptr<FrameInfo> &frameInfo = mFrameInfoQueue.front();
                int ret = mMuxer->muxPacket(move(frameInfo->frame));
                mFrameInfoQueue.pop_front();

                if (ret < 0) {
                    AF_LOGW("muxThreadRun() mMuxer error ret = %d ", ret);

                    //no space error .
                    if (ENOSPC == errno) {
                        hasError = true;
                        sendError(CACHE_ERROR_NO_SPACE);
                        break;
                    }
                }
            }
        }

        if (mInterrupt || mWantStop) {
            AF_LOGW("muxThreadRun() mInterrupt || mWantStop...");
            break;
        }
    }

    int ret = mMuxer->close();
    if (ret < 0) {
        AF_LOGW("muxThreadRun() mMuxer close ret = %d ", ret);
        hasError = true;
        sendError(CACHE_ERROR_MUXER_CLOSE);
    }

    if (hasError) {
        return -1;
    }

    bool muxSuccess = false;
    if (mInterrupt || mWantStop) {
        muxSuccess = false;
    } else if (mFrameEof) {
        muxSuccess = true;
    }

    if (mResultCallback != nullptr) {
        mResultCallback(muxSuccess);
    }

    AF_LOGD("muxThreadRun() end...");
    return -1;
}


void CacheFileRemuxer::stop()
{
    AF_LOGD("CacheFileRemuxer::stop...");
    {
        std::unique_lock<mutex> lock(mThreadMutex);
        mWantStop = true;

        if (mMuxThread != nullptr) {
            mMuxThread->stop();
            delete mMuxThread;
        }

        mMuxThread = nullptr;
    }
    //muxer will close at thread end.
}

void CacheFileRemuxer::interrupt()
{
    std::unique_lock<mutex> lock(mThreadMutex);
    mInterrupt = true;
}

void CacheFileRemuxer::initMuxer()
{
    mMuxer->setCopyPts(false);
    mMuxer->setOpenFunc([this]() -> void {
        AF_LOGD("open dest file");
        mDestFileCntl->openFile();
    });
    mMuxer->setCloseFunc([this]() -> void {
        AF_LOGD("close dest file");
        mDestFileCntl->closeFile();
    });
    mMuxer->setWritePacketCallback(io_write, this);
    mMuxer->setWriteDataTypeCallback(io_write_data_type, this);
    mMuxer->setSeekCallback(io_seek, this);
    mMuxer->setStreamMetas(mStreamMetas);
}


int CacheFileRemuxer::io_write(void *opaque, uint8_t *buf, int size)
{
    auto *cacheFileRemuxer = static_cast<CacheFileRemuxer *>(opaque);
    int ret = cacheFileRemuxer->mDestFileCntl->writeFile(buf, size);
    return ret;
}

int CacheFileRemuxer::io_write_data_type(void *opaque, uint8_t *buf, int size,
        IMuxer::DataType type, int64_t time)
{
    return io_write(opaque, buf, size);
}

int64_t CacheFileRemuxer::io_seek(void *opaque, int64_t offset, int whence)
{
    auto *cacheFileRemuxer = static_cast<CacheFileRemuxer *>(opaque);
    return cacheFileRemuxer->mDestFileCntl->seekFile(offset, whence);
}

void CacheFileRemuxer::setErrorCallback(function<void(int, string)> callback)
{
    mErrorCallback = callback;
}

void CacheFileRemuxer::setResultCallback(function<void(bool)> callback)
{
    mResultCallback = callback;
}

void CacheFileRemuxer::setStreamMeta(const vector<Stream_meta *> *streamMetas)
{
    mStreamMetas = streamMetas;
}

void CacheFileRemuxer::sendError(const CacheRet &ret)
{
    mRemuxSuc = false;

    if (mErrorCallback != nullptr) {
        mErrorCallback(ret.mCode, ret.mMsg);
    }
}

