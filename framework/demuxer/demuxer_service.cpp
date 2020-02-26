//
// Created by moqi on 2018/2/1.
//
#define LOG_TAG "demuxer_service"

#include <utils/frame_work_log.h>
#include <cassert>
#include <utils/errors/framework_error.h>
#include <demuxer/sample_decrypt/SampleDecryptDemuxer.h>
#include "demuxer_service.h"

#define  MAX_PROBE_SIZE 1024

namespace Cicada {

    demuxer_service::demuxer_service(IDataSource *pDataSource)
        : mPDataSource(pDataSource)
    {
    }

    demuxer_service::~demuxer_service()
    {
        delete[] mPProbBuffer;
    }


    void *demuxer_service::getCodecPar(int streamIndex)
    {
        AF_TRACE;

        if (mDemuxerPtr) {
            return mDemuxerPtr->getCodecPar(streamIndex);
        }

        return nullptr;
    }

    void demuxer_service::SetDataCallBack(demuxer_callback_read read, void *read_arg,
                                          demuxer_callback_seek seek,
                                          void *seek_arg, const char *uri)
    {
        AF_TRACE;
        mReadCb = read;
        mReadArg = read_arg;
        mSeekCb = seek;
        mSeekArg = seek_arg;
    }

    void demuxer_service::setDemuxerCb(const std::function<void(std::string, std::string)> &func)
    {
        if (mDemuxerPtr) {
            return mDemuxerPtr->setDemuxerCb(func);
        }

        mDemuxerCbfunc = func;
    }

#define CHECK_DEMUXER do{if (mDemuxerPtr == nullptr) return -1;}while(false);
#define CHECK_DEMUXER_V do{if (mDemuxerPtr == nullptr) return;}while(false);

    int demuxer_service::createDemuxer(demuxer_type type)
    {
        if (mSDec) {
            SampleDecryptDemuxer *demuxer = new SampleDecryptDemuxer();
            demuxer->setDecryptor(mSDec);
            mDemuxerPtr = std::unique_ptr<IDemuxer>(demuxer);
        }

        if (mDemuxerPtr == nullptr) {
            if (mPProbBuffer == nullptr) {
                mPProbBuffer = new uint8_t[MAX_PROBE_SIZE];
                mProbBufferSize = 0;
            }

            if (!mNoFile) {
                while (mProbBufferSize < 128) {
                    int ret = 0;

                    if (mPDataSource) {
                        ret = mPDataSource->Read(mPProbBuffer + mProbBufferSize, MAX_PROBE_SIZE - mProbBufferSize - 1);
                    } else if (mReadCb) {
                        ret = mReadCb(mReadArg, mPProbBuffer + mProbBufferSize, MAX_PROBE_SIZE - mProbBufferSize - 1);
                    } else {
                        return -EINVAL;
                    }

                    if (ret > 0) {
                        mProbBufferSize += ret;
                    } else {
                        break;
                    }
                }

                if (mProbBufferSize <= 0) {
                    return -EIO;
                }
            }

            string url;

            if (mPDataSource) {
                url = mPDataSource->GetUri();
            }

            mPProbBuffer[mProbBufferSize] = 0;
            mDemuxerPtr = std::unique_ptr<IDemuxer>(
                              demuxerPrototype::create(url, mPProbBuffer, mProbBufferSize, move(mDemuxerMeta), mOpts));

            if (!mDemuxerPtr) {
                AF_LOGE("create demuxer error\n");
                return FRAMEWORK_ERR_FORMAT_NOT_SUPPORT;
            }
        }

        mDemuxerPtr->setDemuxerCb(mDemuxerCbfunc);

        if (mDemuxerPtr->isPlayList()) {
            IDataSource::SourceConfig config;

            if (mPDataSource) {
                mPDataSource->Get_config(config);
            }

            mDemuxerPtr->setDataSourceConfig(config);
        }

        return 0;
    }

    int demuxer_service::initOpen(demuxer_type type)
    {
        AF_TRACE;

        if (mDemuxerPtr == nullptr) {
            createDemuxer(type);
        }

        if (!mDemuxerPtr) {
//            AF_LOGE("create demuxer error\n");
            return FRAMEWORK_ERR_FORMAT_NOT_SUPPORT;
        }

        if (!mNoFile) {
            if (mPDataSource == nullptr && mSeekCb == nullptr) {
                AF_LOGD("not support seek\n");
                mDemuxerPtr->SetDataCallBack(read_callback, nullptr, open_callback, interrupt_callback, this);
            } else {
                mDemuxerPtr->SetDataCallBack(read_callback, seek_callback, open_callback, interrupt_callback, this);
            }
        }

        if (mFirstSeekUs > 0) {
            mDemuxerPtr->Seek(mFirstSeekUs, 0, -1);
        }

        int openRet = mDemuxerPtr->Open();
        return openRet;
    }

    int demuxer_service::readPacket(std::unique_ptr<IAFPacket> &packet, int index)
    {
        int ret;
        CHECK_DEMUXER;
        ret = mDemuxerPtr->ReadPacket(packet, index);
        return ret;
    }

    void demuxer_service::close()
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        mDemuxerPtr->Close();
    }

    int demuxer_service::start()
    {
        AF_TRACE;
        CHECK_DEMUXER;
        mDemuxerPtr->Start();
        return 0;
    }

    void demuxer_service::stop()
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        mDemuxerPtr->Stop();
    }

    void demuxer_service::preStop()
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        mDemuxerPtr->PreStop();
    }

    int demuxer_service::GetNbStreams()
    {
        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->GetNbStreams();
    }

    int demuxer_service::GetSourceMeta(Source_meta **meta)
    {
        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->GetSourceMeta(meta);
    }

    int demuxer_service::GetStreamMeta(Stream_meta *meta, int index, bool sub)
    {
//        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->GetStreamMeta(meta, index, sub);
    }

    int demuxer_service::OpenStream(int index)
    {
        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->OpenStream(index);
    }

    void demuxer_service::CloseStream(int index)
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        mDemuxerPtr->CloseStream(index);
    }

    std::string demuxer_service::GetProperty(int index, const string &key)
    {
        AF_TRACE;

        if (nullptr == mDemuxerPtr) {
            return "";
        }

        return mDemuxerPtr->GetProperty(index, key);
    }

    int demuxer_service::SetOption(const std::string &key, const int64_t value)
    {
        if (nullptr != mDemuxerPtr) {
            return mDemuxerPtr->SetOption(key, value);
        }

        return 0;
    }

    bool demuxer_service::isPlayList()
    {
        if (nullptr == mDemuxerPtr) {
            return false;
        }

        return mDemuxerPtr->isPlayList();
    }

    void demuxer_service::flush()
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        mDemuxerPtr->flush();
        //  mDemuxerPtr->Seek(0,0,-1);
    }

    int demuxer_service::SwitchStreamAligned(int from, int to)
    {
        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->SwitchStreamAligned(from, to);
    }

    int demuxer_service::GetNbSubStream(int index)
    {
        AF_TRACE;
        CHECK_DEMUXER;
        return mDemuxerPtr->GetNbSubStreams(index);
    }

    int demuxer_service::GetRemainSegmentCount(int index)
    {
        CHECK_DEMUXER;
        return mDemuxerPtr->GetRemainSegmentCount(index);
    }

    void demuxer_service::interrupt(int inter)
    {
        AF_TRACE;
        CHECK_DEMUXER_V;
        return mDemuxerPtr->interrupt(inter);
    }

    int demuxer_service::Seek(int64_t us, int flags, int index)
    {
        AF_TRACE;

        if (mDemuxerPtr == nullptr) {
            mFirstSeekUs = us;
            return 0;
        } else {
            return mDemuxerPtr->Seek(us, flags, index);
        }
    }

    void demuxer_service::setDemuxerMeta(unique_ptr<DemuxerMeta> &meta)
    {
        mDemuxerMeta = move(meta);
    }

    int demuxer_service::GetStreamMeta(std::unique_ptr<streamMeta> &meta, int index, bool sub)
    {
        Stream_meta Meta;
        int ret = GetStreamMeta(&Meta, index, sub);

        if (ret < 0) {
            return ret;
        }

        meta = std::unique_ptr<streamMeta>(new streamMeta(&Meta));
        return 0;
    }

    IDemuxer *demuxer_service::getDemuxerHandle()
    {
        return mDemuxerPtr.get();
    }

    int demuxer_service::read_callback(void *arg, uint8_t *buffer, int size)
    {
        auto *pHandle = static_cast<demuxer_service *>(arg);

        if (pHandle->mPProbBuffer && pHandle->curPos < pHandle->mProbBufferSize) {
            int readSize = std::min(size, pHandle->mProbBufferSize - (int) pHandle->curPos);
            memcpy(buffer, pHandle->mPProbBuffer + (int) pHandle->curPos, static_cast<size_t>(readSize));
            pHandle->curPos += readSize;

            if (pHandle->curPos >= pHandle->mProbBufferSize) {
                delete[] pHandle->mPProbBuffer;
                pHandle->mPProbBuffer = nullptr;
            }

            return readSize;
        }

        if (pHandle->mReadCb) {
            return pHandle->mReadCb(pHandle->mReadArg, buffer, size);
        }

        return pHandle->mPDataSource->Read(buffer, size);
    }

    int64_t demuxer_service::seek_callback(void *arg, int64_t offset, int whence)
    {
        auto *pHandle = static_cast<demuxer_service *>(arg);

        //   AF_LOGE("test", "%s %lld \n", __func__, offset);
        if (pHandle->mPProbBuffer) {
            delete[] pHandle->mPProbBuffer;
            pHandle->mPProbBuffer = nullptr;
        }

        if (pHandle->mSeekCb) {
            return pHandle->mSeekCb(pHandle->mSeekArg, offset, whence);
        }

        return pHandle->mPDataSource->Seek(offset, whence);
    }

    int demuxer_service::open_callback(void *arg, const char *url, int64_t start, int64_t end)
    {
        auto *pHandle = static_cast<demuxer_service *>(arg);

        if (pHandle->mPProbBuffer) {
            delete[] pHandle->mPProbBuffer;
            pHandle->mPProbBuffer = nullptr;
        }

        pHandle->mPDataSource->setRange(start, end);
        return pHandle->mPDataSource->Open(url);
    }

    void demuxer_service::interrupt_callback(void *arg, int inter)
    {
        auto *pHandle = static_cast<demuxer_service *>(arg);

        if (pHandle->mPDataSource) {
            return pHandle->mPDataSource->Interrupt(static_cast<bool>(inter));
        }
    }
}
